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
  if ($triggering_element == 'marker_gff3_btn') {
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
    $attributes = "ID=$marker_name;Name=$marker_name;dbxref=CarrotOmics:$entity";

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
