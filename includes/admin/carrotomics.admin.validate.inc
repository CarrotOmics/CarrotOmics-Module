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

  // Add a 'Validate CarrotOmics accessions' button
  $form['val_car_acc_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'val_car_acc_btn',
    '#value'  => 'Validate CarrotOmics accessions',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "For stock items using the CarrotOmics database,"
                 . " update the accession in dbxref to match the"
                 . " cvterm and CarrotOmics entity e.g. accession/123456"
                 . "</em></div><br />",
    '#suffix' => "<hr>",
  ];

  // Add a 'Validate public:// fileloc entries' button
  $form['val_fileloc_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'val_fileloc_btn',
    '#value'  => 'Validate public:// chado.fileloc entries',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "Validate all files stored locally using the public:// URL"
                 . " nomenclature that are listed in the chado.fileloc table."
                 . " File presence, read permission, size, and MD5 checksum are validated."
                 . "</em></div><br />",
    '#suffix' => "<hr>",
  ];

  // Add a 'Validate eimage urls' button
  $form['val_eimage_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'val_eimage_btn',
    '#value'  => 'Validate eimage urls',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "Validate all eimage urls to verify file is present"
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
  }
  else if ($triggering_element == 'val_car_acc_btn') {
    [$nerrors, $status] = carrotomics_admin_validate_carrotomics_accessions();
  }
  else if ($triggering_element == 'val_fileloc_btn') {
    [$nerrors, $status] = carrotomics_admin_validate_fileloc();
  }
  else if ($triggering_element == 'val_eimage_btn') {
    [$nerrors, $status] = carrotomics_admin_validate_eimage();
  }
  else {
    [$nerrors, $status] = [1, "Unknown button \"$triggering_element\" was pressed"];
  }
  drupal_set_message($status, $nerrors?'error':'status');
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

/**
 * For germplasm accessions or other stocks using CarrotOmmics as the
 * database, the convention is to have the accession reflect the
 * CarrotOmics type id and entity number, e.g. accession/123456
 * so that links are clickable
 */
function carrotomics_admin_validate_carrotomics_accessions() {

  // Find candidate accession from the CarrotOmics DB
  $sql1 = "SELECT S.stock_id, S.dbxref_id, DBX.accession, CV.name, CB.bundle_id"
        . " FROM {stock} S"
        . " LEFT JOIN {dbxref} DBX ON S.dbxref_id=DBX.dbxref_id"
        . " LEFT JOIN {cvterm} CV ON S.type_id=CV.cvterm_id"
        . " LEFT JOIN [chado_bundle] CB ON S.type_id=CB.type_id"
        . " WHERE DBX.db_id=(SELECT db_id FROM {db} WHERE name='CarrotOmics')";
  $args1 = [];
  try {
    $results1 = chado_query($sql1, $args1);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }

  $ncorrect = 0;
  $nchanged = 0;
  $nunpublished = 0;
  $nerrors = 0;

  while ($obj = $results1->fetchObject()) {
    $stock_id = $obj->stock_id;
    $dbxref_id = $obj->dbxref_id;
    $accession = $obj->accession;
    // need to remove spaces i.e. "generated germplasm"
    $cvname = preg_replace('/\s/', '', $obj->name);
    $bundle_id = $obj->bundle_id;
    if ( $bundle_id ) {
      // Query the appropriate bundle to find the entity
      $sql2 = "SELECT B.entity_id FROM [chado_bio_data_$bundle_id] B WHERE B.record_id=:record";
      $args2 = [ ':record' => $stock_id ];
      try {
        $results2 = chado_query($sql2, $args2);
      } catch (Exception $e) {
        return [1, $e->getMessage()];
      }
      $entity_id = $results2->fetchField();

      // entity is not present if not yet published
      if ( $entity_id ) {
        $expected = $cvname . '/' . $entity_id;
        if ( $accession == $expected ) {
          $ncorrect++;
        }
        else {
          // incorrect, need to update
          $sql3 = "UPDATE {dbxref} SET accession=:acc WHERE dbxref_id=:dbxref_id";
          $args3 = [ ':acc' => $expected, ':dbxref_id' => $dbxref_id ];
          try {
            $results3 = chado_query($sql3, $args3);
          } catch (Exception $e) {
            return [1, $e->getMessage()];
          }
          $nchanged++;
        }
      }
      else {
        $nunpublished++;
      }
    }
    else {
      $nerrors++;  // not expected to occur
    }
  }

  return [$nerrors, "$nchanged accessions updated, $ncorrect were correct, $nunpublished need to be published"];
}

/**
 * Validate files used by the tripal file module
 *
 */
function carrotomics_admin_validate_fileloc() {
  // Ensure that all URLs in the chado.fileloc table using the public://
  // method of access are present. checksums and file sizes are checked
  // for correctness

  // Ihe output message will be a list of problems found.
  // It will be empty if everything is correct.
  $outputmessage = '';
  $nchecked = 0;
  $nerrors = 0;

  // get the base path of where public:// points
  $files_path = variable_get('file_public_path', conf_path() . '/files');

  // get a list of files to check, order by file_id for a somewhat chronological list,
  // then check each one
  $sql = "SELECT file_id, uri, md5checksum, size FROM {fileloc} ORDER BY file_id";
  $results = chado_query($sql, []);
  while ($obj = $results->fetchObject()) {
    $uri = $obj->uri;
    // we only validate local files using public:// prefix
    if (preg_match('|^public://|', $uri)) {
      $nchecked++;
      $file_id = $obj->file_id;
      $md5checksum = $obj->md5checksum;
      $size = $obj->size;
      $real_path = $files_path . preg_replace('|^public://|', '/', $uri);
      $entity = chado_get_record_entity_by_table('file', $file_id);
      $entity_url = "/file/$entity";  // to_do: I bet there's a more correct way to do this
      // 1. validate existence
      if (!file_exists($real_path)) {
        $outputmessage .= "<li>File not present \"<a href=\"$entity_url\">$uri</a>\"</li>";
        $nerrors++;
      }
      // 2. validate file permissions
      elseif (!is_readable($real_path)) {
        $outputmessage .= "<li>File permission problem for \"<a href=\"$entity_url\">$uri</a>\", file is not readable</li>";
        $nerrors++;
      }
      else {
        // 3. validate size
        $actualsize = filesize($real_path);
        if ($size != $actualsize) {
          $outputmessage .= "<li>Size stored for file \"<a href=\"$entity_url\">$uri</a>\" $size is different than actual size $actualsize</li>";
          $nerrors++;
        }
        // 4. warn if no md5 checksum. When missing in chado.fileloc,
        // it is still defined as a string of 32 space characters
        elseif ((!$md5checksum) or (preg_match('/ /', $md5checksum))) {
          $outputmessage .= "<li>There is no valid MD5 checksum stored for file \"<a href=\"$entity_url\">$uri</a>\"</li>";
          $nerrors++;
        }
        else {
          // 5. validate md5 checksum
          try {
            $actualmd5checksum = md5_file($real_path);
            if ($md5checksum != $actualmd5checksum) {
              $outputmessage .= "<li>MD5 checksum \"$md5checksum\" stored for file \"<a href=\"$entity_url\">$uri</a>\" $md5checksum is different than actual checksum \"$actualmd5checksum\"</li>";
              $nerrors++;
            }
          } catch (Exception $e) {
            $outputmessage .= "<li>File read error for file \"<a href=\"$entity_url\">$uri</a>\" ".$e->getMessage()."</li>";
            $nerrors++;
          }
        }
      }
    }
  }

  if ($outputmessage) {
    return [1, "$nerrors errors found<ol>$outputmessage</ol>"];
  }
  else {
    return [0, "$nchecked public:// files checked, there were no problems found"];
  }
}

/**
 * Validate urls in the eimage table
 *
 */
function carrotomics_admin_validate_eimage() {
  // Ensure that all URLs in the chado.eimage table point to an existing file

  // Ihe output message will be a list of problems found.
  // It will be empty if everything is correct.
  $outputmessage = '';
  $nchecked = 0;
  $nerrors = 0;

  // get the base path of where eimage urls start
#/var/www/2020-02-18/drupal-7.69/sites/default/files/bulk_data/carrotomics/germplasm/
  $files_path = variable_get('file_public_path', conf_path() . '/files' ) . '/bulk_data/carrotomics/germplasm';
  $files_path = drupal_realpath($files_path);

  // get a list of files to check, order by file_id for a somewhat chronological list,
  // then check each one
  $sql = "SELECT eimage_id, image_uri FROM {eimage} ORDER BY eimage_id";
  $results = chado_query($sql, []);
  while ($obj = $results->fetchObject()) {
    $eimage_id = $obj->eimage_id;
    $image_uri = $obj->image_uri;
    $nchecked++;
    $full_image_path = $files_path . '/image/' . $image_uri;
    $thumbnail_path = $files_path . '/icon/' . $image_uri;

    // 1. validate full resolution image existence
    if (!file_exists($full_image_path)) {
      $outputmessage .= "<li>Image not present \"$full_image_path\"</li>";
      $nerrors++;
    }
    // 2. validate full resolution image size
    elseif (!filesize($full_image_path)) {
      $outputmessage .= "<li>Zero size image file \"$full_image_path\"</li>";
      $nerrors++;
    }
    // 3. validate thumbnail image existence
    if (!file_exists($thumbnail_path)) {
      $outputmessage .= "<li>Thumbnail not present \"$thumbnail_path\"</li>";
      $nerrors++;
    }
    // 4. validate thumbnail image size
    elseif (!filesize($thumbnail_path)) {
      $outputmessage .= "<li>Zero size thumbnail image file \"$thumbnail_path\"</li>";
      $nerrors++;
    }
    if ($nerrors >= 100) {
      $outputmessage .= "<li value=\"0\">Too many errors, further errors are not shown</li>";
      break;
    }
  }

  if ($outputmessage) {
    return [1, "$nerrors errors found<ol>$outputmessage</ol>"];
  }
  else {
    return [0, "$nchecked files checked, there were no problems found"];
  }
}
