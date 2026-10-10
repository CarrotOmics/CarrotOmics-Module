<?php

namespace Drupal\carrotomics\Form;

use Drupal\Core\Config\ConfigFactory;
use Drupal\Core\File\FileSystem;
use Drupal\Core\Form\FormStateInterface;
use Drupal\pgsql\Driver\Database\pgsql\Connection;
use Drupal\tripal_chado\Database\ChadoConnection;
use Drupal\tripal\Services\TripalEntityLookup;
use Drupal\tripal\Services\TripalLogger;
use Drupal\tripal\TripalBackendPublish\PluginManager\TripalBackendPublishManager;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides the user interface for CarrotOmics publication tools.
 */
class CarrotOmicsAdminExport extends CarrotOmicsAdminFormBase {

  /**
   * Drupal configuration service.
   *
   * @var Drupal\Core\File\FileSystem
   */
  protected FileSystem $file_system;

  /**
   * CarrotOmicsAdminPub class constructor.
   *
   * Prepares injected services.
   */
  public function __construct(
    ConfigFactory $config_factory,
    Connection $drupal_connection,
    ChadoConnection $chado_connection,
    TripalEntityLookup $entity_lookup_manager,
    TripalLogger $logger,
    TripalBackendPublishManager $publish_manager,
    FileSystem $file_system,
  ) {
    parent::__construct($config_factory, $drupal_connection, $chado_connection, $entity_lookup_manager, $logger, $publish_manager);
    $this->file_system = $file_system;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('config.factory'),
      $container->get('database'),
      $container->get('tripal_chado.database'),
      $container->get('tripal.tripal_entity.lookup'),
      $container->get('tripal.logger'),
      $container->get('tripal.backend_publish'),
      $container->get('file_system'),
    );
  }

  /**
   * Returns a unique string identifying the form.
   *
   * @return string
   *   The unique string identifying the form.
   */
  public function getFormId() {
    return 'carrotomics_admin_export_form';
  }

  /**
   * {@inheritdoc}
   *
   * Provides the form definition.
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    // Attach CSS for stying link buttons.
    $form['#attached']['library'][] = 'carrotomics/carrotomics-admin';

    // An element where submit results will go.
    $storage = $form_state->getStorage();
    if (isset($storage['results'])) {
      $form['pub_results'] = [
        '#type'   => 'fieldset',
        '#title' => 'Results',
        '#markup' => '<div class="form-results">' . $storage['results'] . '</div>',
        '#weight' => -100,
      ];
    }

    // Populate pull-down selectors.
    $options_genomes = $this->jbrowseGenomes();

    // Add a 'GBS Marker Locations' button.
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
      '#suffix' => '<hr>',
    ];

    // Add a 'Update stock_featuremap Table' button.
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
      '#suffix' => '<hr>',
    ];

    // Add a 'Export gff3 of all mapped markers' button
    // Define an output file and pass the name through the form.
    $form['marker_gff3_select'] = [
      '#type' => 'select',
      '#title' => 'Analysis or project for JBrowse Instance NOT TESTED',
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
      '#suffix' => '<hr>',
    ];

    // Add a 'Generate a list of all organisms' button.
    $form['org_list_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'org_list_btn',
      '#value'  => 'Generate a list of all organisms',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . "Generate a tsv file of all organism_id numbers and associated NCBI"
      . " and GRIN taxonomy ID numbers currently in CarrotOmics"
      . "</em></div><br />",
      '#suffix' => '<hr>',
    ];

    // Add a 'Generate a list of all projects, analyses, and assays' button.
    $form['paa_list_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'paa_list_btn',
      '#value'  => 'Generate a list of all projects, analyses, and assays',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . "Generate a tsv file of all projects, analyses, and assays"
      . " currently in CarrotOmics"
      . "</em></div><br />",
      '#suffix' => '<hr>',
    ];

    // Add a 'Generate a list of all stocks and linked projects' button.
    $form['carrotomics_stock_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'carrotomics_stock_btn',
      '#value'  => 'Generate a list of all stocks and linked projects',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . "Generate a tsv file of all germplasm accessions (stocks) currently in CarrotOmics,"
      . " and any linked projects"
      . "</em></div><br />",
      '#suffix' => '<hr>',
    ];

    // Add a 'Generate a list of all biomaterials and linked projects
    // and analyses' button.
    $form['biomaterial_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'biomaterial_btn',
      '#value'  => 'Generate a list of all biomaterials and linked projects and analyses',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . "Generate a tsv file of all biomaterials currently in CarrotOmics,"
      . " and any linked projects or analyses"
      . "</em></div><br />",
      '#suffix' => '<hr>',
    ];

    // Add a 'Generate a list of all images of stocks' button.
    $form['stock_image_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'stock_image_btn',
      '#value'  => 'Generate a list of all images of stocks',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . "Generate a tsv file of all images of stocks (usually germplasm accessions)"
      . " currently in CarrotOmics"
      . "</em></div><br />",
      '#suffix' => '<hr>',
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {

    $form_state->setRebuild(TRUE);

    // Gets the triggering element, i.e. which button was pressed.
    $triggering_element = $form_state->getTriggeringElement()['#name'];

    // Gets the values from form input fields.
    $jbrowse_record = $form_state->getValue('marker_gff3_select');

    // Take action depending on which button was pressed.
    $nerrors = 0;
    $status = '';
    if ($triggering_element == 'gbs_loc_btn') {
      [$nerrors, $status] = $this->gbsLocation();
      if (!$nerrors) {
        $this->returnFile('GBS Locations', $form_state);
      }
    }
    elseif ($triggering_element == 'stk_fmp_btn') {
      [$nerrors, $status] = $this->populateStockFeaturemap();
      if (!$nerrors) {
        $this->returnFile('Stock Featuremap', $form_state);
      }
    }
    elseif ($triggering_element == 'marker_gff3_btn') {
      [$nerrors, $status] = $this->markerGff3($jbrowse_record);
      if (!$nerrors) {
        $this->returnFile('JBrowse Analysis', $form_state);
      }
    }
    elseif ($triggering_element == 'org_list_btn') {
      [$nerrors, $status] = $this->organismList();
      if (!$nerrors) {
        $this->returnFile('Organisms', $form_state);
      }
    }
    elseif ($triggering_element == 'paa_list_btn') {
      [$nerrors, $status] = $this->paaList();
      if (!$nerrors) {
        $this->returnFile('PAA List', $form_state);
      }
    }
    elseif ($triggering_element == 'carrotomics_stock_btn') {
      [$nerrors, $status] = $this->carrotomicsStock();
      if (!$nerrors) {
        $this->returnFile('Stocks', $form_state);
      }
    }
    elseif ($triggering_element == 'biomaterial_btn') {
      [$nerrors, $status] = $this->biomaterial();
      if (!$nerrors) {
        $this->returnFile('Biomaterial', $form_state);
      }
    }
    elseif ($triggering_element == 'stock_image_btn') {
      [$nerrors, $status] = $this->stockImage();
      if (!$nerrors) {
        $this->returnFile('Stock Images', $form_state);
      }
    }
    else {
      [$nerrors, $status] = [1, "Unknown button \"$triggering_element\" was pressed"];
    }
    $form_state->setStorage(['results' => $status]);
    if ($nerrors && $nerrors > 0) {
      $this->messenger()->addError($nerrors . ' errors');
    }
  }

  /**
   * Generate genome locations for GBS markers in the chado.featureloc table.
   *
   * It is hard to be sure which genome to use, so this function will just
   * create a table that can then be used to create a MCL file. Populate
   * with default values for DCARv2 genome.
   */
  protected function gbsLocation() {
    // Default value for the "*genome" column.
    $genome = 'Carrot Genome Assembly DCARv2';
    $genus = 'Daucus';
    $species = 'carota';
    $infraspecific_type = 'subspecies';
    $infraspecific_name = 'sativus';

    $headers = ['*genome', '*chromosome', '*marker_name',
      'genus', 'species', 'infraspecific_type', 'infraspecific_name',
      'fmin', 'fmax', 'strand',
    ];

    $sql = "SELECT F.feature_id, F.name FROM {1:feature} F LEFT JOIN {1:featureloc} L"
         . " ON F.feature_id=L.feature_id WHERE F.name ~ '^MK\d+ \d_\d+$' AND L.featureloc_id IS NULL"
         . " AND F.type_id=(SELECT cvterm_id FROM {1:cvterm} C WHERE C.name='genetic_marker'"
         . " AND C.cv_id=(SELECT cv_id FROM {1:cv} WHERE name='sequence'))"
         . " ORDER BY F.name";
    $args = [];
    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (Exception $e) {
      return [1, $e->getMessage()];
    }

    // Initialize the output file content.
    $content = implode("\t", $headers) . "\n";
    $nfound = 0;

    while ($obj = $results->fetchObject()) {
      $nfound++;
      $name = $obj->name;
      // Extract chromosome number and position. strand is left blank.
      preg_match('/(\d+)_(\d+)$/', $name, $matches);
      $chromosome = 'DCARv2_Chr' . $matches[1];
      $fmax = $matches[2];
      $fmin = $fmax - 1;
      $strand = '';
      $cols = [$genome, $chromosome, $name,
        $genus, $species, $infraspecific_type, $infraspecific_name,
        $fmin, $fmax, $strand,
      ];
      $content .= implode("\t", $cols) . "\n";
    }

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('gbsLocation: Exported @nfound GBS marker locations',
      ['@nfound' => $nfound]);

    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as an error will indicate to not
    // return the empty file.
    if (!$nfound) {
      return [-1, 'There were no GBS marker locations found'];
    }
  }

  /**
   * Generate a file to populate the chado.stock_featuremap table.
   *
   * Publications are used to find the links.
   */
  protected function populateStockFeaturemap() {
    // The first three columns are for the chado bulk loader. The other
    // columns are information for the manual filtering process.
    $headers = ['featuremap_id', 'stock_id', 'type_id',
      '(stock type)', '(pub_id)', '(map name)', '(stock name)', '(stock uniquename)',
    ];

    $sql = ' SELECT F.featuremap_id, F.name AS mapname, FP.pub_id, S.name AS stockname, S.uniquename, SP.stock_id, S.type_id, CV.name AS stocktype'
         . ' FROM {1:featuremap} F'
    // Find publication(s) referencing this map.
         . ' LEFT JOIN {1:featuremap_pub} FP ON F.featuremap_id=FP.featuremap_id'
    // Find stocks the publication refers to.
         . ' LEFT JOIN {1:stock_pub} SP ON FP.pub_id=SP.pub_id'
    // Used to get type_id of stock.
         . ' LEFT JOIN {1:stock} S ON SP.stock_id=S.stock_id'
    // Used to get name of type of stock.
         . ' LEFT JOIN {1:cvterm} CV ON S.type_id=CV.cvterm_id'
    // Used to find if already present in stock_featuremap.
         . ' LEFT JOIN {1:stock_featuremap} SF ON SP.stock_id=SF.stock_id'
    // Eliminates when publication does not refer to any stock.
         . ' WHERE SP.pub_ID IS NOT NULL'
    // If not null it is already in stock_featuremap table, can skip.
         . ' AND SF.featuremap_id IS NULL'
         . ' ORDER BY F.featuremap_id, SP.stock_id';
    $args = [];

    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (Exception $e) {
      return [1, $e->getMessage()];
    }

    // Initialize the output file content.
    $content = implode("\t", $headers) . "\n";
    $nfound = 0;
    // Can't easily DISTINCT with so many columns, so remove duplicates
    // in the loop.
    $seen = [];

    while ($obj = $results->fetchObject()) {
      $key = $obj->featuremap_id . ':' . $obj->stock_id . ':' . $obj->type_id;
      if (!array_key_exists($key, $seen)) {
        $cols = [$obj->featuremap_id,
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

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('populateStockFeaturemap: Exported @nfound possible map to stock references',
      ['@nfound' => $nfound]);

    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as an error will indicate to not
    // return the empty file.
    if (!$nfound) {
      return [-1, 'There were no no map to stock references found'];
    }
  }

  /**
   * Get a list of analyses linked to JBrowse instances.
   *
   * This list is used to populate a select pulldown.
   */
  protected function jbrowseGenomes() {
    // The JBrowse instances store entity IDs for content types of either
    // genome project or genome assembly. Convert to analysis_id values.
    $sql = 'SELECT I.uid,'
    . ' A.genome_assembly_name_value AS analysis_name, A.genome_assembly_name_record_id AS analysis_id,'
    . ' P.project_name_value AS project_name, P.project_name_record_id AS project_id'
    . ' FROM {0:jbrowse_instance_field_data} J'
    . ' LEFT JOIN {0:tripal_entity__genome_assembly_name} A ON J.assembly_page_id=A.entity_id'
    . ' LEFT JOIN {0:tripal_entity__project_name} P ON J.assembly_page_id=P.entity_id'
    . ' ORDER BY name';
    $args = [];
    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (\Exception $e) {
      // If tripal_jbrowse is not installed.
      return ['' => 'tripal_jbrowse is not installed'];
    }
    $options_genomes = [];
    while ($obj = $results->fetchObject()) {
      if ($obj->analysis_id) {
        $options_genomes['analysis_id.' . $obj->analysis_id] = $obj->analysis_name;
      }
      elseif ($obj->project_id) {
        $options_genomes['project_id.' . $obj->project_id] = $obj->project_name;
      }
      else {
        throw new \Exception('Unable to get ID from JBrowse instance uid=' . $obj->uid);
      }
    }
    return($options_genomes);
  }

  /**
   * Generate gff3 of all mapped markers with genome locations.
   *
   * @param string $jbrowse_source
   *   The analysis_id or project_id pkey for the select JBrowse
   *   instance, e.g. "project_id.123" or "analysis_id.567".
   */
  protected function markerGff3(string $jbrowse_source) {

    // If tripal_jbrowse is not installed and button clicked anyway.
    if (!$jbrowse_source) {
      return [1, 'tripal_jbrowse is not installed'];
    }

    // Determine whether analysis or project.
    if (preg_match('/^analysis_id.(\d+)$/', $jbrowse_source, $matches)) {
      $type = 'analysis';
      $pkey = $matches[1];
    }
    elseif (preg_match('/^project_id.(\d+)$/', $jbrowse_source, $matches)) {
      $type = 'project';
      $pkey = $matches[1];
    }
    else {
      throw new \Exception('Unable to parse jbrowse_source: ' . $jbrowse_source);
    }
    // @todo cannot currently handle project!
    if ($type == 'project') {
      return [1, 'Using project not yet implemented'];
    }
    $sql = "SELECT
      MLF.name as marker_locus_name, MLF.uniquename as marker_locus_uniquename,
        MLF.feature_id as marker_locus_feature_id,
      GMF.name AS genetic_marker_name, GMF.uniquename as genetic_marker_uniquename,
        GMF.feature_id as genetic_marker_feature_id,
      FL.fmin, FL.fmax, FL.strand,
      CHR.name AS seqname, CHR.uniquename AS seq_uniquename,
      AF.analysis_id AS genome_analysis_id
    FROM {1:feature} MLF
    INNER JOIN {1:feature_relationship} FR
      ON FR.subject_id = MLF.feature_id
      AND MLF.type_id = (SELECT cvterm_id FROM {1:cvterm} WHERE name = 'marker_locus'
        AND cv_id = (SELECT cv_id FROM {1:cv} WHERE name = 'MAIN'))
      AND FR.type_id = (SELECT cvterm_id FROM {cvterm} WHERE cvterm.name = 'instance_of'
        AND cv_id = (SELECT cv_id FROM {1:cv} WHERE name = 'relationship'))
    INNER JOIN feature GMF
      ON FR.object_id = GMF.feature_id
      AND GMF.type_id = (SELECT cvterm_id FROM {1:cvterm} WHERE name = 'genetic_marker'
        AND cv_id = (SELECT cv_id FROM {1:cv} WHERE name = 'sequence'))
      AND FR.type_id = (SELECT cvterm_id FROM {1:cvterm} WHERE name = 'instance_of'
        AND cv_id = (SELECT cv_id FROM {1:cv} WHERE name = 'relationship'))
    INNER JOIN {1:featureloc} FL
      ON FL.feature_id = GMF.feature_id
    INNER JOIN {1:feature} CHR
      ON CHR.feature_id = FL.srcfeature_id
    INNER JOIN {1:analysisfeature} AF
      ON CHR.feature_id = AF.feature_id
    WHERE AF.analysis_id = :genome_analysis_id
    ORDER BY CHR.uniquename, FL.fmin, FL.fmax";

    $args = [':genome_analysis_id' => $pkey];
    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (Exception $e) {
      return [1, $e->getMessage()];
    }

    // Initialize the output file content.
    $content = "##gff-version 3\n";
    $nfound = 0;

    while ($obj = $results->fetchObject()) {
      $nfound++;

      // Look up entity for this marker.
      $entity = $this->entity_lookup_manager->getEntityId($obj->genetic_marker_feature_id, NULL, NULL, 'feature');

      // Use the "short" name as Name, "long" name (unique) as ID.
      $marker_unique_name = $obj->marker_locus_uniquename;
      $marker_name = $obj->marker_locus_name;
      $chr = $obj->seq_uniquename;
      // Fmin is 0-based.
      $start = $obj->fmin + 1;
      $end = $obj->fmax;
      $score = '.';
      $strand = $obj->strand;
      $phase = '.';
      if (!$strand) {
        $strand = '.';
      }
      $attributes = "ID=$marker_unique_name;Name=$marker_name;dbxref=CarrotOmics:$entity";

      // Columns are: [0]seqid [1]source [2]type [3]start [4]end
      // [5]score [6]strand [7]phase [8]attribues.
      $cols = [$chr, 'mapped_markers', 'genetic_marker',
        $start, $end, $score, $strand, $phase,
        $attributes,
      ];
      $content .= implode("\t", $cols) . "\n";
    }

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('markerGff3: Exported @nfound markers with genome locations',
      ['@nfound' => $nfound]);

    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as an error will indicate to not
    // return the empty file.
    if (!$nfound) {
      return [-1, 'There were no markers found with genome locations'];
    }
  }

  /**
   * Get a list of organisms currently in CarrotOmics.
   */
  public function organismList() {
    $nfound = 0;

    // Lookup NCBITaxon database.
    $query1 = $this->chado_connection->select('1:db', 'D');
    $query1->fields('D', ['db_id']);
    $query1->condition('D.name', 'NCBITaxon', '=');
    $ncbi_taxon_id = $query1->execute()->fetchField();
    if (!$ncbi_taxon_id) {
      return [1, 'Error, no db record for NCBITaxon, cannot continue'];
    }

    // Lookup GRINTaxon database.
    $query1 = $this->chado_connection->select('1:db', 'D');
    $query1->fields('D', ['db_id']);
    $query1->condition('D.name', 'GRINTaxon', '=');
    $grin_taxon_id = $query1->execute()->fetchField();
    if (!$grin_taxon_id) {
      return [1, 'Error, no db record for GRINTaxon, cannot continue'];
    }

    // Select all organisms, they may or may not have taxid values.
    $sql = "SELECT O.organism_id,"
         . " (SELECT accession FROM {1:dbxref} D1 JOIN {1:organism_dbxref} OD1 ON OD1.dbxref_id=D1.dbxref_id"
         . "   WHERE OD1.organism_id=O.organism_ID AND D1.db_id=(SELECT db_id FROM {1:db} WHERE name='NCBITaxon'))"
         . " AS ncbi_taxid,"
         . " (SELECT accession FROM {1:dbxref} D2 JOIN {1:organism_dbxref} OD2 ON OD2.dbxref_id=D2.dbxref_id"
         . "   WHERE OD2.organism_id=O.organism_ID AND D2.db_id=(SELECT db_id FROM {1:db} WHERE name='GRINTaxon'))"
         . " AS grin_taxid,"
         . " O.genus, O.species,"
         . " (SELECT name FROM {1:cvterm} where cvterm_id=O.type_id) AS infraspecific_type,"
         . " O.infraspecific_name, O.common_name, O.abbreviation, O.comment"
         . " FROM {1:organism} O ORDER BY O.organism_id";
    try {
      $results = $this->chado_connection->query($sql, []);
    }
    catch (Exception $e) {
      return [1, $e->getMessage()];
    }

    // Not including comment because it may contain line breaks.
    $headers = [
      'organism_id',
      'ncbi_taxid',
      'grin_taxid',
      'genus',
      'species',
      'infraspecific_type',
      'infraspecific_name',
      'common_name',
      'abbreviation',
    ];
    $content = implode("\t", $headers) . "\n";
    foreach ($results as $obj) {
      $itype = $obj->infraspecific_type;
      if ($itype == 'no_rank') {
        $itype = '';
      }
      $cols = [
        $obj->organism_id,
        $obj->ncbi_taxid,
        $obj->grin_taxid,
        $obj->genus,
        $obj->species,
        $itype,
        $obj->infraspecific_name,
        $obj->common_name,
        $obj->abbreviation,
      ];
      $content .= implode("\t", $cols) . "\n";
      $nfound++;
    }

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('organismList: Exported @nfound organisms',
      ['@nfound' => $nfound]);

    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as an error will indicate to not
    // return the empty file.
    if (!$nfound) {
      return [-1, 'There were no organisms found'];
    }
  }

  /**
   * Get a list of projects, analyses, and assays currently in CarrotOmics.
   */
  protected function paaList() {
    $nfound = 0;

    // Combine all tables that are in the general category of "analysis"
    // (i.e. analysis, assay, project) into one master list.
    $sql = "SELECT 'project' AS table, PR.project_id AS record_id, PP1.value AS type, PR.name,"
         . " CASE WHEN PR.description = '' OR PR.description IS NULL THEN PP2.value ELSE PR.description END AS description"
         . " FROM {1:project} PR"
         . " LEFT JOIN {1:projectprop} PP1 ON PR.project_id=PP1.project_id"
         . "  AND PP1.type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='project_type' AND cv_id=(SELECT cv_id FROM {1:cv} WHERE name='MAIN'))"
         . " LEFT JOIN {1:projectprop} PP2 ON PR.project_id=PP2.project_id"
         . "  AND PP2.type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='description' AND cv_id=(SELECT cv_id FROM {1:cv} WHERE name='MAIN'))"
         . " UNION ALL "
         . "SELECT 'analysis' AS table, AN.analysis_id AS record_id, ANP.value AS type, AN.name, AN.description"
         . " FROM {1:analysis} AN"
         . " LEFT JOIN {1:analysisprop} ANP ON AN.analysis_id=ANP.analysis_id"
         . "  AND ANP.type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='type' AND cv_id=(SELECT cv_id FROM {1:cv} WHERE name='rdfs'))"
         . " UNION ALL "
         . "SELECT 'assay' AS table, AY.assay_id AS record_id, AYP.value AS type, AY.name, AY.description"
         . " FROM {1:assay} AY"
         . " LEFT JOIN {1:assayprop} AYP ON AY.assay_id=AYP.assay_id"
         . "  AND AYP.type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='type' AND cv_id=(SELECT cv_id FROM {1:cv} WHERE name='rdfs'))"
         . " UNION ALL "
         . "SELECT 'study' AS table, ST.study_id AS record_id, STP.value AS type, ST.name, ST.description"
         . " FROM {1:study} ST"
         . " LEFT JOIN {1:studyprop} STP ON ST.study_id=STP.study_id"
         . "  AND STP.type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='type' AND cv_id=(SELECT cv_id FROM {1:cv} WHERE name='rdfs'))"
         . " ORDER BY name";
    $args = [];
    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (Exception $e) {
      return [1, $e->getMessage()];
    }
    $content = implode("\t", ['table', 'record_id', 'type', 'name', 'description']) . "\n";
    while ($obj = $results->fetchObject()) {
      // In case there are any line breaks, remove them.
      $name = preg_replace('/[\n\r]+/', ' ', $obj->name);
      $description = $obj->description ?? '';
      $description = preg_replace('/[\n\r]+/', ' ', $description);
      $cols = [$obj->table, $obj->record_id, $obj->type, $name, $description];
      $content .= implode("\t", $cols) . "\n";
      $nfound++;
    }

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('paaList: Exported @nfound Project, Analysis, and Assay records',
      ['@nfound' => $nfound]);

    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as an error will indicate to not
    // return the empty file.
    if (!$nfound) {
      return [-1, 'There were no Project, Analysis, or Assay records found'];
    }
  }

  /**
   * Get a list of GRIN germplasm accessions (stocks) and linked projects.
   */
  protected function carrotomicsStock() {
    $nfound = 0;

    $sql = "SELECT S.stock_id, DB.name as repository, S.uniquename AS accession,"
         . " P.name AS project FROM {1:stock} S"
         . " LEFT JOIN {1:stock_dbxref} X ON S.stock_id=X.stock_id"
         . " LEFT JOIN {1:dbxref} D ON X.dbxref_id=D.dbxref_id"
         . " LEFT JOIN {1:db} DB ON D.db_id=DB.db_id"
         . " LEFT JOIN {1:project_stock} PS ON S.stock_id=PS.stock_id"
         . " LEFT JOIN {1:project} P ON PS.project_id=P.project_id"
         . " ORDER BY DB.name, S.uniquename, P.name";
    $args = [];
    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (Exception $e) {
      return [1, $e->getMessage()];
    }
    $content = implode("\t", ['stock_id', 'repository', 'accession', 'project']) . "\n";
    while ($obj = $results->fetchObject()) {
      $cols = [$obj->stock_id, $obj->repository, $obj->accession, $obj->project];
      $content .= implode("\t", $cols) . "\n";
      $nfound++;
    }

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('carrotomicsStock: Exported @nfound CarrotOmics stock records',
      ['@nfound' => $nfound]);

    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as an error will indicate to not
    // return the empty file.
    if (!$nfound) {
      return [-1, 'There were no stock records found'];
    }
  }

  /**
   * Get a list of biomaterials (BioSamples) and linked projects, etc.
   */
  protected function biomaterial() {
    $nfound = 0;

    $sql = "SELECT B.name AS biomaterialname, P.name AS projectname, A.name AS analysisname,"
         . " S.name AS assayname FROM {1:biomaterial} B"
         . " LEFT JOIN {1:biomaterial_project} BP ON B.biomaterial_id=BP.biomaterial_id"
         . " LEFT JOIN {1:biomaterial_analysis} BA ON B.biomaterial_id=BA.biomaterial_id"
         . " LEFT JOIN {1:assay_biomaterial} BS ON B.biomaterial_id=BS.biomaterial_id"
         . " LEFT JOIN {1:project} P ON BP.project_id=P.project_id"
         . " LEFT JOIN {1:analysis} A ON BA.analysis_id=A.analysis_id"
         . " LEFT JOIN {1:assay} S ON BS.assay_id=S.assay_id"
         . " ORDER BY B.name";
    $args = [];
    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (Exception $e) {
      return [1, $e->getMessage()];
    }
    $content = implode("\t", ['biomaterialname', 'projectname', 'analysisname', 'assayname']) . "\n";
    while ($obj = $results->fetchObject()) {
      $cols = [$obj->biomaterialname, $obj->projectname, $obj->analysisname, $obj->assayname];
      $content .= implode("\t", $cols) . "\n";
      $nfound++;
    }

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('biomaterial: Exported @nfound biomaterial records',
      ['@nfound' => $nfound]);

    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as an error will indicate to not
    // return the empty file.
    if (!$nfound) {
      return [-1, 'There were no biomaterial records found'];
    }
  }

  /**
   * Get a list of images of stocks.
   */
  protected function stockImage() {
    $nfound = 0;

    $sql = "SELECT I.image_uri, S.uniquename AS accession FROM {1:eimage} I"
         . " LEFT JOIN {1:stock_image} SI ON I.eimage_id=SI.eimage_id"
         . " LEFT JOIN {1:stock} S ON SI.stock_id=S.stock_id"
         . " ORDER BY S.uniquename, I.image_uri";
    $args = [];
    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (Exception $e) {
      return [1, $e->getMessage()];
    }
    $content = implode("\t", ['accession', 'image_uri']) . "\n";
    while ($obj = $results->fetchObject()) {
      $cols = [$obj->accession, $obj->image_uri];
      $content .= implode("\t", $cols) . "\n";
      $nfound++;
    }

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('stockImage: Exported @nfound stock image records',
      ['@nfound' => $nfound]);

    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as an error will indicate to not
    // return the empty file.
    if (!$nfound) {
      return [-1, 'There were no stock image records found'];
    }
  }

}
