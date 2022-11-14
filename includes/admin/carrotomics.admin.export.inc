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

  // Add a 'Generate a list of all organisms' button
  $organism_list_filename = 'public://mcl/user/'.$user->name.'/organism_list.tsv';
  $organism_list_url = file_create_url($organism_list_filename);
  $form['organism_list_filename'] = [
    '#type' => 'hidden',
    '#value' => $organism_list_filename,
  ];
  $form['org_list_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'org_list_btn',
    '#value'  => 'Generate a list of all organisms',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "Generate a tsv file of all organism_id numbers and associated NCBI"
                 . " and GRIN taxonomy ID numbers currently in CarrotOmics"
                 . "</em></div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$organism_list_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  // Add a 'Generate a list of all GRIN stocks and linked projects' button
  $grin_stock_filename = 'public://mcl/user/'.$user->name.'/grin_stock.tsv';
  $grin_stock_url = file_create_url($grin_stock_filename);
  $form['grin_stock_filename'] = [
    '#type' => 'hidden',
    '#value' => $grin_stock_filename,
  ];
  $form['grin_stock_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'grin_stock_btn',
    '#value'  => 'Generate a list of all GRIN stocks and linked projects',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "Generate a tsv file of all GRIN germplasm accessions (stocks) currently in CarrotOmics,"
                 . " and any linked projects"
                 . "</em></div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$grin_stock_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  // Add a 'Generate a list of all images of stocks' button
  $stock_image_filename = 'public://mcl/user/'.$user->name.'/stock_image.tsv';
  $stock_image_url = file_create_url($stock_image_filename);
  $form['stock_image_filename'] = [
    '#type' => 'hidden',
    '#value' => $stock_image_filename,
  ];
  $form['stock_image_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'stock_image_btn',
    '#value'  => 'Generate a list of all images of stocks',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "Generate a tsv file of all images of stocks (usually germplasm accessions)"
                 . " currently in CarrotOmics"
                 . "</em></div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$stock_image_url."\">here</a></div><br />"
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
  else if ($triggering_element == 'org_list_btn') {
    $organism_list_filename = $form_state['values']['organism_list_filename'];
    [$nerrors, $status] = carrotomics_admin_organism_list($organism_list_filename);
  }
  else if ($triggering_element == 'grin_stock_btn') {
    $grin_stock_filename = $form_state['values']['grin_stock_filename'];
    [$nerrors, $status] = carrotomics_admin_grin_stock($grin_stock_filename);
  }
  else if ($triggering_element == 'stock_image_btn') {
    $stock_image_filename = $form_state['values']['stock_image_filename'];
    [$nerrors, $status] = carrotomics_admin_stock_image($stock_image_filename);
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

/**
 * Get a list of organisms currently in carrotomics
 *
 */
function carrotomics_admin_organism_list($organism_list_filename) {
  $nfound = 0;

  // All organisms, they may or may not have taxid values
  $sql = "SELECT O.organism_id,"
       . " (SELECT accession FROM {dbxref} D1 JOIN {organism_dbxref} OD1 ON OD1.dbxref_id=D1.dbxref_id"
       . "   WHERE OD1.organism_id=O.organism_ID AND D1.db_id=(SELECT db_id FROM {db} WHERE name='NCBITaxon'))"
       . " AS ncbi_taxid,"
       . " (SELECT accession FROM {dbxref} D2 JOIN {organism_dbxref} OD2 ON OD2.dbxref_id=D2.dbxref_id"
       . "   WHERE OD2.organism_id=O.organism_ID AND D2.db_id=(SELECT db_id FROM {db} WHERE name='GRINTaxon'))"
       . " AS grin_taxid,"
       . " O.genus, O.species,"
       . " (SELECT name FROM {cvterm} where cvterm_id=O.type_id) AS infraspecific_type, O.infraspecific_name,"
       . " O.common_name, O.abbreviation, O.comment"
       . " FROM {organism} O ORDER BY O.organism_id";
  $args = [];
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }
  // Not including comment because it may contain line breaks
  $content = implode("\t", ['organism_id', 'ncbi_taxid', 'grin_taxid',
                            'genus', 'species', 'infraspecific_type', 'infraspecific_name',
                            'common_name', 'abbreviation']) . "\n";
  while ($obj = $results->fetchObject()) {
    $itype = $obj->infraspecific_type;
    if ($itype == 'no_rank') {
      $itype = '';
    }
    $cols = [ $obj->organism_id, $obj->ncbi_taxid, $obj->grin_taxid,
              $obj->genus, $obj->species, $itype, $obj->infraspecific_name,
              $obj->common_name, $obj->abbreviation];
    $content .= implode("\t", $cols) . "\n";
    $nfound++;
  }
  // save file in mcl directory. Save even if nothing retrieved, to overwrite old versions
  file_put_contents($organism_list_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound organisms saved, download them from the link below the button"];
  }
  else {
    return [1, "There are no organisms, this should not happen"];
  }
}

/**
 * Get a list of GRIN germplasm accessions (stocks) and linked projects
 *
 */
function carrotomics_admin_grin_stock($grin_stock_filename) {
  $nfound = 0;

  $sql = "SELECT S.uniquename AS accession, P.name AS project FROM {stock} S"
       . " LEFT JOIN {stock_dbxref} X ON S.stock_id=X.stock_id"
       . " LEFT JOIN {dbxref} D ON X.dbxref_id=D.dbxref_id"
       . " LEFT JOIN {db} DB ON D.db_id=DB.db_id"
       . " LEFT JOIN {project_stock} PS ON S.stock_id=PS.stock_id"
       . " LEFT JOIN {project} P ON PS.project_id=P.project_id"
       . " WHERE DB.name = 'GRIN Accession'"
       . " ORDER BY S.uniquename, P.name";
  $args = [];
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }
  $content = implode("\t", ['accession', 'project']) . "\n";
  while ($obj = $results->fetchObject()) {
    $cols = [$obj->accession, $obj->project];
    $content .= implode("\t", $cols) . "\n";
    $nfound++;
  }
  // save file in mcl directory
  file_put_contents($grin_stock_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound records saved, download them from the link below the button"];
  }
  else {
    return [1, "There are no GRIN accessions, this should not happen"];
  }
}

/**
 * Get a list of images of stocks
 *
 */
function carrotomics_admin_stock_image($stock_image_filename) {
  $nfound = 0;

  $sql = "SELECT I.image_uri, S.uniquename AS accession FROM {eimage} I"
       . " LEFT JOIN {stock_image} SI ON I.eimage_id=SI.eimage_id"
       . " LEFT JOIN {stock} S ON SI.stock_id=S.stock_id"
       . " ORDER BY S.uniquename, I.image_uri";
  $args = [];
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }
  $content = implode("\t", ['accession', 'image_uri']) . "\n";
  while ($obj = $results->fetchObject()) {
    $cols = [$obj->accession, $obj->image_uri];
    $content .= implode("\t", $cols) . "\n";
    $nfound++;
  }
  // save file in mcl directory
  file_put_contents($stock_image_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound records saved, download them from the link below the button"];
  }
  else {
    return [1, "There are no GRIN accessions, this should not happen"];
  }
}
