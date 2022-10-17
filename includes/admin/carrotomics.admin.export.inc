<?php
/**
 * Custom functions for the CarrotOmics site
 *
 * @param $form
 * @param $form_state
 *
 * @ingroup carrotomics_admin
 */
function carrotomics_admin_export_form($form, &$form_state) {

  // Set the breadcrumb
  $breadcrumb = [];
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('CarrotOmics', 'admin/carrotomics/export');
  drupal_set_breadcrumb($breadcrumb);

  global $user;  // used for output file location

  // Create the form
  $form = [];

  // Populate pull-down selectors
  $options_genomes = carrotomics_admin_jbrowse_genomes();

  // Add a 'GBS Marker Locations' button
  $marker_loc_filename = 'public://mcl/user/'.$user->name.'/marker_loc.tsv';
  $marker_loc_url = file_create_url($marker_loc_filename);
  $form['gbs_loc_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'gbs_loc_btn',
    '#value'  => 'GBS Marker Locations',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "For all GBS markers that do not have a genome location,"
                 . " generate a Mainlab Chado Loader data matrix to use for the"
                 . " marker_loc sheet, generated from all GBS markers that use the"
                 . " naming scheme </em>MK012345 1_2345678<em>"
                 . "</em></div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$marker_loc_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  // Add a 'Update stock_featuremap Table' button
  $stock_featuremap_filename = 'public://mcl/user/'.$user->name.'/stock_featuremap.tsv';
  $stock_featuremap_url = file_create_url($stock_featuremap_filename);
  $form['stk_fmp_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'stk_fmp_btn',
    '#value'  => 'Update stock_featuremap Table',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "Use publication links to populate the </em>chado.stock_featuremap<em>"
                 . " table. A publication may link to both a featuremap, as well as to one"
                 . " or more stocks, which can be a population or parents of that population."
                 . " The generated list must be manually curated before loading, because a publication"
                 . " can link to more than one map, or a map may be referenced by multiple publications,"
                 . " so extraneous items must be removed from the list."
                 . "</em></div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$stock_featuremap_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  // Add a 'Export gff3 of all mapped markers' button
  // Define an output file and pass the name through the form
  $marker_gff3_filename = 'public://mcl/user/'.$user->name.'/mapped_markers.gff3';
  $marker_gff3_url = file_create_url($marker_gff3_filename);
  $form['marker_gff3_filename'] = [
    '#type' => 'hidden',
    '#value' => $marker_gff3_filename,
  ];
  $form['marker_gff3_select'] = [
    '#type' => 'select',
    '#title' => 'Analysis for JBrowse Instance',
    '#options' => $options_genomes,
    '#multiple' => FALSE,
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "Generate a gff3 format file of all markers present on linkage maps"
                 . " that have a known genome location. This can then be loaded into JBrowse."
                 . "</em></div><br />",
  ];
  $form['marker_gff3_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'marker_gff3_btn',
    '#value'  => 'Export gff3 of all mapped markers',
    '#suffix' => "<div>Download the generated file from <a href=\"".$marker_gff3_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  return $form;
}

/**
 * Validates the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_export_form_validate($form, &$form_state) {
  // currently only buttons, so no validation is needed
}

/**
 * Submits the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_export_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;

  // Gets the trigger element, i.e. which button was pressed
  $triggering_element = $form_state['triggering_element']['#name'];

  // Take action depending on which button was pressed
  if ($triggering_element == 'gbs_loc_btn') {
    [$nerrors, $status] = carrotomics_admin_gbs_location();
  }
  else if ($triggering_element == 'stk_fmp_btn') {
    [$nerrors, $status] = carrotomics_admin_populate_stock_featuremap();
  }
  else if ($triggering_element == 'marker_gff3_btn') {
    $jbrowse_analysis = $form_state['values']['marker_gff3_select'];
    $marker_gff3_filename = $form_state['values']['marker_gff3_filename'];
    [$nerrors, $status] = carrotomics_admin_marker_gff3($jbrowse_analysis, $marker_gff3_filename);
  }
  else {
    [$nerrors, $status] = [1, "Unknown button \"$triggering_element\" was pressed"];
  }
  drupal_set_message($status, $nerrors?'error':'status');
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
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }

  // initialize the output file content
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
    $cols = [ $genome, $chromosome, $name,
              $genus, $species, $infraspecific_type, $infraspecific_name,
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
 * Generate a file to populate the chado.stock_featuremap table using publications to find the links.
 *
 */
function carrotomics_admin_populate_stock_featuremap() {
  // The first three columns are for the chado bulk loader. The other columns are
  // information for the manual filtering process
  $headers = ['featuremap_id', 'stock_id', 'type_id',
              '(stock type)', '(pub_id)', '(map name)', '(stock name)', '(stock uniquename)'];

  $sql = ' SELECT F.featuremap_id, F.name AS mapname, FP.pub_id, S.name AS stockname, S.uniquename, SP.stock_id, S.type_id, CV.name AS stocktype'
       . ' FROM {featuremap} F'
       . ' LEFT JOIN {featuremap_pub} FP ON F.featuremap_id=FP.featuremap_id'  // find publication(s) referencing this map
       . ' LEFT JOIN {stock_pub} SP ON FP.pub_id=SP.pub_id'  // find stocks the publication refers to
       . ' LEFT JOIN {stock} S ON SP.stock_id=S.stock_id'  // used to get type_id of stock
       . ' LEFT JOIN {cvterm} CV ON S.type_id=CV.cvterm_id'  // used to get name of type of stock
       . ' LEFT JOIN {stock_featuremap} SF ON SP.stock_id=SF.stock_id'  // used to find if already present in stock_featuremap
       . ' WHERE SP.pub_ID IS NOT NULL'  // eliminates when publication does not refer to any stock
       . ' AND SF.featuremap_id IS NULL'  // if not null it is already in stock_featuremap table, can skip
       . ' ORDER BY F.featuremap_id, SP.stock_id';
  $args = [];

  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return[1, $e->getMessage()];
  }

  // initialize the output file content
  $content = implode("\t", $headers) . "\n";
  $nfound = 0;
  $seen = [];  // I can't easily DISTINCT with so many columns, remove duplicates in the loop

  while ($obj = $results->fetchObject()) {
    $key = $obj->featuremap_id . ':' . $obj->stock_id . ':' . $obj->type_id;
    if (!array_key_exists($key, $seen)) {
      $cols = [ $obj->featuremap_id,
                $obj->stock_id,
                $obj->type_id,
                $obj->stocktype,
                $obj->pub_id,
                $obj->mapname,
                $obj->stockname,
                $obj->uniquename,
              ];
      $content .= implode("\t", $cols) . "\n";
      $nfound++;
      $seen[$key] = 1;
    }
  }

  // save file in mcl directory. Save even if nothing retrieved, to overwrite old versions
  global $user;
  $marker_loc_filename = 'public://mcl/user/'.$user->name.'/stock_featuremap.tsv';
  file_put_contents($marker_loc_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound possible map to stock references found, download them from the link below the button"];
  }
  else {
    return [0, "There were no map to stock references found"];
  }
}

/**
 * Get a list of analyses linked to JBrowse instances to use to populate pulldown
 *
 */
function carrotomics_admin_jbrowse_genomes() {
  $sql = 'SELECT A.analysis_id, A.name FROM [tripal_jbrowse_mgmt_instances] JB'
       . ' LEFT JOIN {analysis} A ON JB.analysis_id=A.analysis_id'
       . ' ORDER BY A.name';
  $args = [];
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }
  $options_genomes = [];
  while ($obj = $results->fetchObject()) {
    $options_genomes[$obj->analysis_id] = $obj->name;
  }
  return($options_genomes);
}

/**
 * Generate gff3 of all mapped markers with genome locations
 *
 */
function carrotomics_admin_marker_gff3($jbrowse_analysis, $marker_gff3_filename) {

  $sql = "SELECT
    MLF.name as marker_locus_name, MLF.uniquename as marker_locus_uniquename,
      MLF.feature_id as marker_locus_feature_id,
    GMF.name AS genetic_marker_name, GMF.uniquename as genetic_marker_uniquename,
      GMF.feature_id as genetic_marker_feature_id,
    FL.fmin, FL.fmax, FL.strand,
    CHR.name AS seqname, CHR.uniquename AS seq_uniquename,
    AF.analysis_id AS genome_analysis_id
  FROM {feature} MLF
  INNER JOIN {feature_relationship} FR
    ON FR.subject_id = MLF.feature_id
    AND MLF.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'marker_locus'
      AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN'))
    AND FR.type_id = (SELECT cvterm_id FROM {cvterm} WHERE cvterm.name = 'instance_of'
      AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'relationship'))
  INNER JOIN feature GMF
    ON FR.object_id = GMF.feature_id
    AND GMF.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'genetic_marker'
      AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'sequence'))
    AND FR.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'instance_of'
      AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'relationship'))
  INNER JOIN {featureloc} FL
    ON FL.feature_id = GMF.feature_id
  INNER JOIN {feature} CHR
    ON CHR.feature_id = FL.srcfeature_id
  INNER JOIN {analysisfeature} AF
    ON CHR.feature_id = AF.feature_id
  WHERE AF.analysis_id = :genome_analysis_id
  ORDER BY CHR.uniquename, FL.fmin, FL.fmax";

  $args = [':genome_analysis_id' => $jbrowse_analysis];
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }

  // initialize the output file content
  $content = "##gff-version 3\n";
  $nfound = 0;

  while ($obj = $results->fetchObject()) {
    $nfound++;

    // Look up entity for this marker
    $entity = chado_get_record_entity_by_table('feature', $obj->genetic_marker_feature_id);

    // Use the "short" name
    $marker_unique_name = $obj->marker_locus_uniquename;
    $marker_name = $obj->marker_locus_name;
    $chr = $obj->seq_uniquename;
    // fmin is 0-based
    $start = $obj->fmin + 1;
    $end = $obj->fmax;
    $score = '.';
    $strand = $obj->strand;
    $phase = '.';
    if (!$strand) {
      $strand = '.';
    }
    $attributes = "ID=$marker_unique_name;Name=$marker_name;dbxref=CarrotOmics:$entity";

    // [0]seqid [1]source [2]type [3]start [4]end [5]score [6]strand [7]phase [8]attribues
    $cols = [ $chr, 'mapped_markers', 'genetic_marker',
              $start, $end, $score, $strand, $phase,
              $attributes ];
    $content .= implode("\t", $cols) . "\n";
  }

  // save file in mcl directory. Save even if nothing retrieved, to overwrite old versions
  file_put_contents($marker_gff3_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound markers found, download them from the link below the button"];
  }
  else {
    return [0, "There were no markers found with genome locations"];
  }
}
