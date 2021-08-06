<?php
/**
 * Configure CarrotOmics module
 *
 * @param $form
 * @param $form_state
 *
 * @ingroup carrotomics_admin
 */
function carrotomics_admin_sql_form($form, &$form_state) {

  // Set the breadcrumb
  $breadcrumb = [];
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('CarrotOmics', 'admin/carrotomics/sql');
  drupal_set_breadcrumb($breadcrumb);

  // Create the form
  $form = [];

  // Add a 'Generate publication URLs' button
  $form['pub_url_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'pub_url_btn',
    '#value'  => 'Generate publication URLs',
    '#prefix' => '<div><em>This button will generate a URL for any publication lacking one,'
                 . ' if it has a DOI defined. The URL will be constructed by concatenating'
                 . ' "https://dx.doi.org/" with the DOI.<br />For example, for the DOI'
                 . ' "10.1038/ng.3565" the URL becomes "https://dx.doi.org/10.1038/ng.3565"</em></div><br />',
    '#suffix' => "<hr>",
  ];

  // Add a 'Clean MD5 Checksums' button
  $form['clean_md5_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'clean_md5_btn',
    '#value'  => 'Clean MD5 Checksums',
    '#prefix' => "<div><em>Remove MD5 checksums of null strings from the feature table, these"
                 . " have the value d41d8cd98f00b204e9800998ecf8427e</em></div><br />",
    '#suffix' => "<hr>",
  ];

  return $form;
}

/**
 * Validates the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_sql_form_validate($form, &$form_state) {
  // currently only buttons, so no validation is needed
}

/**
 * Submits the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_sql_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;

  // Gets the trigger element, i.e. which button was pressed
  $triggering_element = $form_state['triggering_element']['#name'];

  // If the 'Generate publication URLs' button was clicked.
  if ($triggering_element == 'pub_url_btn') {
    [$nerrors, $status] = carrotomics_admin_add_doi_url();
    drupal_set_message($status, $nerrors?'error':'status');
  }
  else if ($triggering_element == 'clean_md5_btn') {
    [$nerrors, $status] = carrotomics_admin_remove_useless_checksums();
    drupal_set_message($status, $nerrors?'error':'status');
  }
  else {
    drupal_set_message("Unknown button was pressed", 'error');
  }
}


/**
 * For every publication that does not currently have a URL, see if it has
 * a DOI, and if so, construct a URL based on the DOI with
 *   $url = 'https://dx.doi.org/' . $doi;
 *
 */
function carrotomics_admin_add_doi_url() {
  $errors = '';

  // get cvterm_id to use for URL as type_id in pubprop table
  $cv_name = 'tripal_pub';
  $cvterm_name = 'URL';
  $sql = "SELECT cvterm_id FROM {cvterm} WHERE name = :cvterm_name AND cv_id=(SELECT cv_id FROM {cv} WHERE name = :cv_name)";
  $typeresults = chado_query($sql, [':cv_name' => $cv_name, ':cvterm_name' => $cvterm_name]);
  $obj = $typeresults->fetchObject();
  if ($obj) {
    $type_id = $obj->cvterm_id;
  }
  else {
    $status = t('Error retrieving cvterm_id for "@cvterm_name" from cv "@cv_name"',
                ['@cvterm_name' => $cvterm_name, '@cv_name' => $cv_name]);
    return [1, $status];
  }

  // retrieve an array of all existing URL entries, key is pub_id
  $sql = "SELECT pub_id, value FROM {pubprop} WHERE type_id=$type_id";
  $urlresults = chado_query($sql, []);
  $urls = [];
  while ($obj = $urlresults->fetchObject()) {
    $urls[$obj->pub_id] = $obj->value;
  }

  // retrieve an array of all existing DOI entries, key is pub_id
  $sql = "SELECT pub_id, value FROM {pubprop} WHERE type_id=(SELECT cvterm_id FROM {cvterm} WHERE name='DOI' AND cv_id=(SELECT cv_id FROM {cv} WHERE name='tripal_pub'))";
  $doiresults = chado_query($sql, []);
  $dois = [];
  while ($obj = $doiresults->fetchObject()) {
    $dois[$obj->pub_id] = $obj->value;
  }

  // loop to add missing URLs
  $rank = 0;  // all added URLs will have this rank value. Since they are new, there is no possibility of conflict
  $nadded = 0;
  $nerrors = 0;
  foreach($dois as $pub_id => $doi) {
    // skip if this pub already has a URL
    if (!array_key_exists($pub_id, $urls)) {
      // skip any unusual DOI entries, e.g. OCLC:ocm49858565
      if ( preg_match('/^10\./', $doi) ) {
        // here we generate a new URL and add it
        $url = 'https://dx.doi.org/' . $doi;
        // equivalent SQL: INSERT INTO {pubprop} (pub_id, type_id, value, rank) VALUES ($pub_id, $type_id, $url, $rank);
        $values = [ 'pub_id' => $pub_id,
                    'type_id' => $type_id,
                    'value' => $url,
                    'rank' => $rank ];
        try {
          $result = chado_insert_record('pubprop', $values);
          if ($result) {
            $nadded++;
          }
          else {
            $nerrors++;
          }
        } catch (Exception $e) {
          $nerrors++;
          $errors .= '<br />' . $e->getMessage();
        }
      }
    }
  }

  $status = t('Added @nadded URLs, @nerrors errors', ['@nadded' => $nadded, '@nerrors' => $nerrors]);
  return [$nerrors, $status . $errors];
}

/**
 * Many features get a md5 checksum generated for a null string.
 * These are not useful. This function will remove them.
 *
 */
function carrotomics_admin_remove_useless_checksums() {
  // equivalent SQL: UPDATE chado.feature SET md5checksum = NULL WHERE md5checksum = 'd41d8cd98f00b204e9800998ecf8427e';

  $where = [ 'md5checksum' => 'd41d8cd98f00b204e9800998ecf8427e' ];
  $newvalues = [ 'md5checksum' => NULL ];

  // First get number of matching records that we will update, since
  // chado_update_record only returns true or false
  $resultarray = chado_select_record('feature', [], $where);
  $nchanged = sizeof($resultarray);

  // Now make the changes
  try {
    $success = chado_update_record('feature', $where, $newvalues);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }
  if ($success) {
    return [0, "$nchanged records updated"];
  }
  else {
    return [1, "$nchanged records to update, but something unexpected went wrong"];
  }
}
