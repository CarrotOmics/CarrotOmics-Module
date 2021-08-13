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

  // Add a 'GBS Marker Genome Locations' button
  global $user;
  $marker_loc_filename = 'public://mcl/user/'.$user->name.'/marker_loc.tsv';
  $marker_loc_url = file_create_url($marker_loc_filename);
  $form['gbs_location_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'gbs_location_btn',
    '#value'  => 'GBS Marker Locations',
    '#prefix' => "<div><em>For all GBS markers that do not have a genome location,"
                 . " generate a Mainlab Chado Loader data matrix to use for the"
                 . " marker_loc sheet, generated from all GBS markers that use the"
                 . " naming scheme </em>MK012345 1_2345678</div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$marker_loc_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  // Add a 'Generate QTL Abbreviations' button
  $form['qtl_abbrev_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'qtl_abbrev_btn',
    '#value'  => 'Generate QTL Abbreviations',
    '#prefix' => "<div><em>Generate featureprop entries with MAIN::published_symbol cvterm"
                 . " for all QTL lacking them, if they are of the format </em>MK012345 anytext</em></div><br />",
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
  else if ($triggering_element == 'gbs_location_btn') {
    [$nerrors, $status] = carrotomics_admin_gbs_location();
    drupal_set_message($status, $nerrors?'error':'status');
  }
  else if ($triggering_element == 'qtl_abbrev_btn') {
    [$nerrors, $status] = carrotomics_admin_qtl_abbrev();
    drupal_set_message($status, $nerrors?'error':'status');
  }
  else {
    drupal_set_message("Unknown button was pressed", 'error');
  }
}


/**
 * Looks up a cvterm_id
 */
function carrotomics_admin_lookup_cvterm_id($cv_name, $cvterm_name) {
  $sql = "SELECT cvterm_id FROM {cvterm} WHERE name = :cvterm_name AND cv_id=(SELECT cv_id FROM {cv} WHERE name = :cv_name)";
  $typeresults = chado_query($sql, [':cv_name' => $cv_name, ':cvterm_name' => $cvterm_name]);
  $obj = $typeresults->fetchObject();
  if ($obj) {
    return [0, $obj->cvterm_id];
  }
  else {
    return [1, t('Error retrieving cvterm_id for "@cvterm_name" from cv "@cv_name"',
                 ['@cvterm_name' => $cvterm_name, '@cv_name' => $cv_name])];
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
  [$error, $type_id] = carrotomics_admin_lookup_cvterm_id('tripal_pub', 'URL');
  if ($error ){ return[$error, $type_id]; }

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

/**
 * Generate genome locations for GBS markers in the chado.featureloc table
 *
 */
function carrotomics_admin_gbs_location() {
  // It is hard to be sure which genome to use, so this function will just create
  // a table, that can then be used to create a MCL file. Populate with
  // default values for DCARv2 genome.
  $genome = 'Carrot Genome Assembly DCARv2';  // default value for the "*genome" column
  $genus = 'Daucus';
  $species = 'carota';
  $infraspecific_type = 'subspecies';
  $infraspecific_name = 'sativus';
  $headers = [ '*genome', '*chromosome', '*marker_name',
               'genus', 'species', 'infraspecific_type', 'infraspecific_name',
               'fmin', 'fmax', 'strand' ];

  $sql = "SELECT F.feature_id, F.name FROM {feature} F LEFT JOIN {featureloc} L"
       . " ON F.feature_id=L.feature_id WHERE F.name ~ '^MK\d+ \d_\d+$' AND L.featureloc_id IS NULL"
       . " AND F.type_id=(SELECT cvterm_id FROM cvterm C WHERE C.name='genetic_marker'"
       . " AND C.cv_id=(SELECT cv_id FROM cv WHERE name='sequence'))"
       . " ORDER BY F.name";
  $args = [];
  $results = chado_query($sql, $args);

  // initialize the output file conent
  $content = implode("\t", $headers) . "\n";
  $nfound = 0;

  while ($obj = $results->fetchObject()) {
    $nfound++;
    $name = $obj->name;
    // extract chromosome number and position. strand is left blank
    preg_match('/(\d+)_(\d+)$/', $name, $matches);
    $chromosome = 'DCARv2_Chr' . $matches[1];
    $fmax = $matches[2];
    $fmin = $fmax - 1;
    $strand = '';
    $cols = [ $genome, $chromosome, $name, $genus, $species, $infraspecific_type, $infraspecific_name,
              $fmin, $fmax, $strand ];
    $content .= implode("\t", $cols) . "\n";
  }

  // save file in mcl directory. Save even if nothing retrieved, to overwrite old versions
  global $user;
  $marker_loc_filename = 'public://mcl/user/'.$user->name.'/marker_loc.tsv';
  file_put_contents($marker_loc_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound markers found, download them from the link below the button"];
  }
  else {
    return [0, "There were no markers found without genome locations"];
  }
}

/**
 * For every publication that does not currently have a URL, see if it has
 * a DOI, and if so, construct a URL based on the DOI with
 *   $url = 'https://dx.doi.org/' . $doi;
 *
 */
function carrotomics_admin_qtl_abbrev() {
  $errors = '';

  // get cvterm_id for QTL
  [$error, $qtl_type_id] = carrotomics_admin_lookup_cvterm_id('sequence', 'QTL');
  if ($error ){ return[$error, $qtl_type_id]; }

  // get cvterm_id to use for featureprop table from MAIN::published_symbol
  [$error, $symbol_type_id] = carrotomics_admin_lookup_cvterm_id('MAIN', 'published_symbol');
  if ($error ){ return[$error, $symbol_type_id]; }

  // retrieve an array of all QTL lacking the published_symbol property
  // the query retrieves all QTL featureprop, and in the loop flag those
  // that already have a published_symbol
  $sql = "SELECT DISTINCT F.feature_id, F.name, F.uniquename, FP.type_id FROM {feature} F"
       . " LEFT JOIN {featureprop} FP ON F.feature_id=FP.feature_id"
       . " WHERE F.type_id=:qtl_type_id";
  $args = [':qtl_type_id' => $qtl_type_id];
  $results = chado_query($sql, $args);
  $qtls = [];
  // we will often iterate the same feature_id multiple times due to multiple featureprop records
  while ($obj = $results->fetchObject()) {
    $qtls[$obj->feature_id]['name'] = $obj->uniquename;
    if ($obj->type_id == $symbol_type_id) {
      $qtls[$obj->feature_id]['exclude'] = 1;
    }
  }

  // loop to add missing published_symbol entries to the featureprop table
  $rank = 0;  // all added lines will have this rank value. Since they are new, there is no possibility of conflict
  $nadded = 0;
  $nskipped = 0;
  $nerrors = 0;
  foreach($qtls as $feature_id => $record) {
    $uniquename = $record['name'];
    $exclude = array_key_exists('exclude', $record);  // defined only when excluded
    // skip if excluded because it already has a published_symbol
    if (!$exclude) {
      // Derive name by removing the 'MK012345 ' portion. If that's missing, skip it
      if ( preg_match('/^MK\d{6} (.*)/', $uniquename, $matches) ) {
        $published_symbol = $matches[1];
        if ($published_symbol) {
          $values = [ 'feature_id' => $feature_id,
                      'type_id' => $symbol_type_id,
                      'value' => $published_symbol,
                      'rank' => $rank ];
          try {
            $result = chado_insert_record('featureprop', $values);
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
        else {
          $errors .= "Unparseable name \"$uniquename\"<br>";
          $nerrors++;
        }
      }
      else {
        $nskipped++;
      }
    }
  }

  $status = t('Added @nadded published_names, @nskipped skipped, @nerrors errors',
              ['@nadded' => $nadded, '@nskipped' => $nskipped, '@nerrors' => $nerrors]);
  return [$nerrors, $status . $errors];
}
