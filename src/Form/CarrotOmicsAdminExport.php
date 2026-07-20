<?php

namespace Drupal\carrotomics\Form;

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
    Connection $drupal_connection,
    ChadoConnection $chado_connection,
    TripalEntityLookup $entity_lookup_manager,
    TripalLogger $logger,
    TripalBackendPublishManager $publish_manager,
    FileSystem $file_system,
  ) {
    parent::__construct($drupal_connection, $chado_connection, $entity_lookup_manager, $logger, $publish_manager);
    $this->file_system = $file_system;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
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

    // ;;;
    if (FALSE) {
      // Populate pull-down selectors.
      $options_genomes = carrotomics_admin_jbrowse_genomes();

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
        '#suffix' => '<hr>',
      ];

    } //;;;
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

    // ;;;
    if (FALSE) {
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

    } //;;;
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {

    $form_state->setRebuild(TRUE);

    // Gets the triggering element, i.e. which button was pressed.
    $triggering_element = $form_state->getTriggeringElement()['#name'];

    // Take action depending on which button was pressed.
    $nerrors = 0;
    $status = '';
    if ($triggering_element == 'gbs_loc_btn') {
      [$nerrors, $status] = carrotomics_admin_gbs_location();
    }
    elseif ($triggering_element == 'stk_fmp_btn') {
      [$nerrors, $status] = carrotomics_admin_populate_stock_featuremap();
    }
    elseif ($triggering_element == 'marker_gff3_btn') {
      $jbrowse_analysis = $form_state['values']['marker_gff3_select'];
      $marker_gff3_filename = $form_state['values']['marker_gff3_filename'];
      [$nerrors, $status] = carrotomics_admin_marker_gff3($jbrowse_analysis, $marker_gff3_filename);
    }
    elseif ($triggering_element == 'org_list_btn') {
      [$nerrors, $status] = $this->organismList();
      if (!$nerrors) {
        $this->returnFile('Organisms', $form_state);
      }
    }
    elseif ($triggering_element == 'paa_list_btn') {
      $paa_list_filename = $form_state['values']['paa_list_filename'];
      [$nerrors, $status] = carrotomics_admin_paa_list($paa_list_filename);
    }
    elseif ($triggering_element == 'carrotomics_stock_btn') {
      $carrotomics_stock_filename = $form_state['values']['carrotomics_stock_filename'];
      [$nerrors, $status] = carrotomics_admin_carrotomics_stock($carrotomics_stock_filename);
    }
    elseif ($triggering_element == 'biomaterial_btn') {
      $biomaterial_filename = $form_state['values']['biomaterial_filename'];
      [$nerrors, $status] = carrotomics_admin_biomaterial($biomaterial_filename);
    }
    elseif ($triggering_element == 'stock_image_btn') {
      $stock_image_filename = $form_state['values']['stock_image_filename'];
      [$nerrors, $status] = carrotomics_admin_stock_image($stock_image_filename);
    }
    else {
      [$nerrors, $status] = [1, "Unknown button \"$triggering_element\" was pressed"];
    }
    $form_state->setStorage(['results' => $status]);
    if ($nerrors) {
      $this->messenger()->addError($nerrors . ' errors');
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
         . " (SELECT accession FROM {dbxref} D2 JOIN {organism_dbxref} OD2 ON OD2.dbxref_id=D2.dbxref_id"
         . "   WHERE OD2.organism_id=O.organism_ID AND D2.db_id=(SELECT db_id FROM {1:db} WHERE name='GRINTaxon'))"
         . " AS grin_taxid,"
         . " O.genus, O.species,"
         . " (SELECT name FROM {cvterm} where cvterm_id=O.type_id) AS infraspecific_type, O.infraspecific_name,"
         . " O.common_name, O.abbreviation, O.comment"
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
      return [1, 'There were no organisms found'];
    }
  }

}
