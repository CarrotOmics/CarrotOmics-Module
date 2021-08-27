<?php
/**
 * Configure CarrotOmics module
 *
 * @param $form
 * @param $form_state
 *
 * @ingroup carrotomics_admin
 */
function carrotomics_admin_validate_form($form, &$form_state) {

  // Set the breadcrumb
  $breadcrumb = [];
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('CarrotOmics', 'admin/carrotomics/validate');
  drupal_set_breadcrumb($breadcrumb);

  // Create the form
  $form = [];

  // Add a 'Find Unpublished Content' button
  $form['find_unpub_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'find_unpub_btn',
    '#value'  => 'Find Unpublished Content',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "List any Tripal content types that have unpublished content"
                 . "</em></div><br />",
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
function carrotomics_admin_validate_form_validate($form, &$form_state) {
}

/**
 * Submits the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_validate_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;

  // Gets the trigger element.
  $triggering_element = $form_state['triggering_element']['#name'];

  if ($triggering_element == 'find_unpub_btn') {
    [$nerrors, $status] = carrotomics_admin_find_unpublished();
    drupal_set_message($status, $nerrors?'error':'status');
  }
  else {
    drupal_set_message("Unknown button was pressed", 'error');
  }
}

/**
 * Find any tripal content types that need to be published.
 *
 */
function carrotomics_admin_find_unpublished() {
  // This is based on the chado_publish_records() function in
  // tripal/tripal_chado/api/tripal_chado.api.inc, but we
  // only go as far as getting the count of unpublished records

  // first get a list of defined bio_data bundles
  $sql = "SELECT name, label FROM [tripal_bundle] ORDER BY term_id";
  $results = chado_query($sql, []);
  $bundle_names = [];
  while ($obj = $results->fetchObject()) {
    $bundle_names[$obj->name] = $obj->label;
  }

  // the output message will be a list of content types that need to
  // be published, it will be empty if everything is already published
  $outputmessage = '';

  foreach($bundle_names as $bundle_name => $bundle_label) {
    // The following section of code is copied from
    // tripal/tripal_chado/api/tripal_chado.api.inc function chado_publish_records()
    // up to the point where we determine the number of publishable records

    // Start an array for caching objects to save performance.
    $cache = [];

    // Load the bundle entity so we can get information about which Chado
    // table/field this entity belongs to.
    $bundle = tripal_load_bundle_entity(['name' => $bundle_name]);
    $cache['bundle'] = $bundle;

    if (!$bundle) {
      tripal_report_error($message_type, TRIPAL_ERROR,
        "Unknown bundle. Could not publish record: @error",
        ['@error' => 'The bundle name must be provided'], $message_opts);
      return FALSE;
    }
    $chado_entity_table = chado_get_bundle_entity_table($bundle);

    // Get the mapping of the bio data type to the Chado table.
    $chado_bundle = db_select('chado_bundle', 'cb')
      ->fields('cb')
      ->condition('bundle_id', $bundle->id)
      ->execute()
      ->fetchObject();
    if (!$chado_bundle) {
      tripal_report_error('tripal_chado', TRIPAL_ERROR,
        "Cannot find mapping of bundle to Chado tables. Could not publish record.");
      return FALSE;
    }

    // Load the term for use in setting the alias for each entity created.
    $term = entity_load('TripalTerm', ['id' => $bundle->term_id]);
    $cache['term'] = $term;

    $table = $chado_bundle->data_table;
    $type_column = $chado_bundle->type_column;
    $type_linker_table = $chado_bundle->type_linker_table;
    $cvterm_id = $chado_bundle->type_id;
    $type_value = $chado_bundle->type_value;

    // Get the table information for the Chado table.
    $table_schema = chado_get_schema($table);
    $pkey_field = $table_schema['primary key'][0];

    // Construct the SQL for identifying which records should be published.
    $args = [];
    $select = "SELECT T.$pkey_field as record_id ";
    $from = "
      FROM {" . $table . "} T
        LEFT JOIN [" . $chado_entity_table . "] CE on CE.record_id = T.$pkey_field
    ";

    $where = " WHERE CE.record_id IS NULL ";

    // Handle records that are mapped to property tables.
    if ($type_linker_table and $type_column and $type_value) {
      $propschema = chado_get_schema($type_linker_table);
      $fkeys = $propschema['foreign keys'][$table]['columns'];
      foreach ($fkeys as $leftkey => $rightkey) {
        if ($rightkey == $pkey_field) {
          $from .= " INNER JOIN {" . $type_linker_table . "} LT ON T.$pkey_field = LT.$leftkey ";
        }
      }
      $where .= "AND LT.$type_column = :cvterm_id and LT.value = :prop_value";
      $args[':cvterm_id'] = $cvterm_id;
      $args[':prop_value'] = $type_value;
    }

    // Handle records that are mapped to cvterm linking tables.
    if ($type_linker_table and $type_column and !$type_value) {
      $cvtschema = chado_get_schema($type_linker_table);
      $fkeys = $cvtschema['foreign keys'][$table]['columns'];
      foreach ($fkeys as $leftkey => $rightkey) {
        if ($rightkey == $pkey_field) {
          $from .= " INNER JOIN {" . $type_linker_table . "} LT ON T.$pkey_field = LT.$leftkey ";
        }
      }
      $where .= "AND LT.$type_column = :cvterm_id";
      $args[':cvterm_id'] = $cvterm_id;
    }

    // Handle records that are mapped via a type_id column in the base table.
    if (!$type_linker_table and $type_column) {
      $where .= "AND T.$type_column = :cvterm_id";
      $args[':cvterm_id'] = $cvterm_id;
    }

    // Handle the case where records are in the cvterm table and mapped via a single
    // vocab.  Here we use the type_value for the cv_id.
    if ($table == 'cvterm' and $type_value) {
      $where .= "AND T.cv_id = :cv_id";
      $args[':cv_id'] = $type_value;
    }

    // Handle the case where records are in the cvterm table but we want to
    // use all of the child terms.
    if ($table == 'cvterm' and !$type_value) {
      $where .= "AND T.cvterm_id IN (
         SELECT CVTP.subject_id
         FROM {cvtermpath} CVTP
         WHERE CVTP.object_id = :cvterm_id)
       ";
      $args[':cvterm_id'] = $cvterm_id;
    }

    // First get the count
    $sql = "SELECT count(*) as num_records " . $from . $where;
    $result = chado_query($sql, $args);
    $count = $result->fetchField();

    // end of copied code

    // Add this bundle to the output message only if $count is > 0
    if ($count) {   
      $outputmessage .= "There are $count records to publish for \"$bundle_label\" ($bundle_name)<br>";
    }
  }

  if ($outputmessage) {
    return [1, $outputmessage];
  }
  else {
    return [0, "There are no tripal content types that need to be published"];
  }
}
