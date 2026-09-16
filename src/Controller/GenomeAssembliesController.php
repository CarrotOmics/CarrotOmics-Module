<?php

namespace Drupal\carrotomics\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Link;
use Drupal\Core\Url;
use Drupal\tripal\Services\TripalEntityLookup;
use Drupal\tripal_chado\Database\ChadoConnection;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Generates the list of genome assemblies page.
 */

class GenomeAssembliesController extends ControllerBase {

  /**
   * The Chado database connection.
   *
   * @var Drupal\tripal_chado\Database\ChadoConnection
   */
  protected ChadoConnection $chado_connection;

  /**
   * Tripal entity lookup service.
   *
   * @var Drupal\tripal\Services\TripalEntityLookup
   */
  protected TripalEntityLookup $entity_lookup_manager;

  /**
   * Controller constructor.
   */
  public function __construct(ChadoConnection $chado_connection, TripalEntityLookup $entity_lookup_manager) {
    $this->chado_connection = $chado_connection;
    $this->entity_lookup_manager = $entity_lookup_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container): static {
    return new static(
      $container->get('tripal_chado.database'),
      $container->get('tripal.tripal_entity.lookup'),
    );
  }

  /**
   * Generates the CarrotOmics Assembly List page.
   *
   * @return array
   *   The content render array.
   */
  public function content(): array {

    $elements = [];

    /**
     * Lookup CV terms we will use in the queries.
     */
    $terms = [
      'cd' => 'EDAM:Core data',
      'ga' => 'EDAM:Sequence assembly (genome assembly)',
    ];
    $cvterm_ids = [];
    foreach ($terms as $short => $term) {
      [$cv, $name] = explode(':', $term, 2);
      $term_query = $this->chado_connection->select('1:cvterm', 'T');
      $term_query->leftJoin('1:cv', 'V', '[T].cv_id=[V].cv_id');
      $term_query->condition('T.name', $name, '=');
      $term_query->condition('V.name', $cv, '=');
      $term_query->addField('T', 'cvterm_id', 'cvterm_id');
      $cvterm_id = $term_query->execute()->fetchField();
      if (!$cvterm_id) {
        throw new \Exception("GenomeAssembliesController: Term \"$term\" not found");
      }
      $cvterm_ids[$short] = $cvterm_id;
    }

    /**
     * Lookup and store all analyses that are genome assemblies.
     *
     * Genome assembly is any analysis having the property
     * 'EDAM:Sequence assembly (genome assembly)' with a value of
     * 'genome_assembly'.
     * Types are distinguised by the property 'EDAM:Core data'.
     * This property will have a value of either 'mitochondrial_assembly'
     * or 'plastid_assembly', or 'core_data' for integrated assemblies.
     * Other genome assemblies do not have this property.
     *
     * Get the entity_id from a drupal field table because this is too
     * costly to get from the entity lookup service for this many records.
     */
    $analyses = [];
    $analysis_query = $this->chado_connection->select('1:analysis', 'A');
    $analysis_query->leftJoin('1:analysisprop', 'P', '[A].analysis_id=[P].analysis_id');
    $analysis_query->leftJoin('0:tripal_entity__genome_assembly_program_version', 'D', '[A].analysis_id=[D].genome_assembly_program_version_record_id');
    $analysis_query->condition('P.type_id', [$cvterm_ids['ga'], $cvterm_ids['cd']], 'IN');
    $analysis_query->addField('A', 'analysis_id', 'analysis_id');
    $analysis_query->addField('A', 'name', 'name');
    $analysis_query->addField('P', 'type_id', 'type_id');
    $analysis_query->addField('P', 'value', 'value');
    $analysis_query->addField('D', 'entity_id', 'entity_id');
    $analysis_query->orderBy('A.timeexecuted');
    $analysis_results = $analysis_query->execute();
    while ($result = $analysis_results->fetchObject()) {
      $short = array_search($result->type_id, $cvterm_ids);
      $entity_id = $result->entity_id;
      $analyses[$result->analysis_id][$short] = [$result->value, $result->name, $entity_id];
    }

    /**
     * Genome assemblies integrated into CarrotOmics.
     *
     * These assemblies are defined by having an analysisprop
     * with cv term 'EDAM:Core data' and value 'integrated_assembly'.
     */
    $ia_items = [];
    foreach ($analyses as $analysis_id => $info) {
      if (($info['ga'][0] ?? '') == 'genome_assembly') {
        if (($info['cd'][0] ?? '') == 'integrated_assembly') {
          $name = $info['ga'][1];
          $entity_id = $info['ga'][2];
          if ($entity_id) {
            // n.b. this is too slow for this many records:
            // $name = Link::fromTextAndUrl($name, Url::fromUserInput('/bio_data/' . $entity_id))->toString();
            $name = ['#markup' => '<a href="/bio_data/' . $entity_id . '">' . $name . '</a>'];
          }
          $ia_items[] = $name;
        }
      }
    }
    $elements['integrated_assemblies'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Genome assemblies integrated into CarrotOmics'),
      'body' => [
        '#theme' => 'item_list',
        '#list_type' => 'ol',
        '#header' => [],
        '#items' => $ia_items,
      ],
    ];

    /**
     * Other assemblies referenced in CarrotOmics.
     */
    $oa_items = [];
    foreach ($analyses as $analysis_id => $info) {
      if (($info['ga'][0] ?? '') == 'genome_assembly') {
        if (!($info['cd'][0] ?? '')) {
          $name = $info['ga'][1];
          $entity_id = $info['ga'][2];
          if ($entity_id) {
            $name = ['#markup' => '<a href="/bio_data/' . $entity_id . '">' . $name . '</a>'];
          }
          $oa_items[] = $name;
        }
      }
    }
    $elements['other_assemblies'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Other assemblies referenced in CarrotOmics'),
      'body' => [
        '#theme' => 'item_list',
        '#list_type' => 'ol',
        '#header' => [],
        '#items' => $oa_items,
      ],
    ];

    /**
     * Mitochondrial assemblies integrated into CarrotOmics.
     */
    $ma_items = [];
    foreach ($analyses as $analysis_id => $info) {
      if (($info['ga'][0] ?? '') == 'genome_assembly') {
        if (($info['cd'][0] ?? '') == 'mitochondrial_assembly') {
          $name = $info['ga'][1];
          $entity_id = $info['ga'][2];
          if ($entity_id) {
            $name = ['#markup' => '<a href="/bio_data/' . $entity_id . '">' . $name . '</a>'];
          }
          $ma_items[] = $name;
        }
      }
    }
    $elements['mitochondrial_assemblies'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Mitochondrial assemblies integrated into CarrotOmics'),
      'body' => [
        '#theme' => 'item_list',
        '#list_type' => 'ol',
        '#header' => [],
        '#items' => $ma_items,
      ],
    ];

    /**
     * Plastid assemblies integrated into CarrotOmics.
     */
    $pa_items = [];
    foreach ($analyses as $analysis_id => $info) {
      if (($info['ga'][0] ?? '') == 'genome_assembly') {
        if (($info['cd'][0] ?? '') == 'plastid_assembly') {
          $name = $info['ga'][1];
          $entity_id = $info['ga'][2];
          if ($entity_id) {
            $name = ['#markup' => '<a href="/bio_data/' . $entity_id . '">' . $name . '</a>'];
          }
          $pa_items[] = $name;
        }
      }
    }
    $elements['plastid_assemblies'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Plastid assemblies integrated into CarrotOmics'),
      'body' => [
        '#theme' => 'item_list',
        '#list_type' => 'ol',
        '#header' => [],
        '#items' => $pa_items,
      ],
    ];

    return $elements;
  }

}
