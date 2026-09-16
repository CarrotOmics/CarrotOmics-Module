<?php

namespace Drupal\carrotomics\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Link;
use Drupal\Core\Url;
use Drupal\tripal\Services\TripalEntityLookup;
use Drupal\tripal_chado\Database\ChadoConnection;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Generates the Publications with Phylogenetic Trees page.
 */

class PhylogeneticTreesController extends ControllerBase {

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

    /**
     * Get the list of trees.
     */
    $phylotrees = [];
    $phylotree_query = $this->chado_connection->select('1:phylotree', 'T');
    $phylotree_query->leftJoin('1:analysis', 'A', '[T].analysis_id=[A].analysis_id');
    $phylotree_query->leftJoin('1:analysis_pub', 'AP', '[A].analysis_id=[AP].analysis_id');
    $phylotree_query->leftJoin('1:pub', 'P', '[AP].pub_id=[P].pub_id');
    $phylotree_query->leftJoin('0:tripal_entity__phylotree_dbxref', 'D1', '[T].phylotree_id=[D1].phylotree_dbxref_record_id');
    $phylotree_query->leftJoin('0:tripal_entity__pub_uniquename', 'D2', '[P].pub_id=[D2].pub_uniquename_record_id');
    $phylotree_query->addField('T', 'name', 'tree_name');
    $phylotree_query->addField('P', 'pub_id', 'pub_id');
    $phylotree_query->addField('P', 'pyear', 'pyear');
    $phylotree_query->addField('P', 'title', 'pub_title');
    $phylotree_query->addField('D1', 'entity_id', 'tree_entity_id');
    $phylotree_query->addField('D2', 'entity_id', 'pub_entity_id');
    $phylotree_query->orderBy('P.pyear');
    $phylotree_query->orderBy('P.title');
    $phylotree_query->orderBy('T.name');
    $phylotree_results = $phylotree_query->execute();
    while ($result = $phylotree_results->fetchObject()) {
      $pub_id = $result->pub_id;
      $pyear = $result->pyear;
      $pub_title = $result->pub_title;
      if ($result->pub_entity_id) {
        $pub_title = ['#markup' => '<a href="/bio_data/' . $result->pub_entity_id . '">' . $pub_title . '</a>'];
      }
      $tree_name = $result->tree_name;
      if ($result->tree_entity_id) {
        $tree_name = ['#markup' => '<a href="/bio_data/' . $result->tree_entity_id . '">' . $tree_name . '</a>'];
      }
      if ($pub_title) {
        $phylotrees[$pyear][$pub_id]['pub'] = $pub_title;
        $phylotrees[$pyear][$pub_id]['trees'][] = $tree_name;
      }
    }

    // Collapse tree arrays into ordered lists in the third column.
    $rows = [];
    foreach ($phylotrees as $pyear => $level1) {
      foreach (array_keys($level1) as $pub_id) {
        $tree_ol = [
          '#theme' => 'item_list',
          '#list_type' => 'ol',
          '#items' => $level1[$pub_id]['trees'],
        ];
        $rows[] = [
          $pyear,
          'pub' => ['data' => $level1[$pub_id]['pub']],
          'trees' => ['data' => $tree_ol],
        ];
      }
    }

    // The render array.
    $elements = [];
    $elements['#attached']['library'][] = 'carrotomics/olivero_carrotomics';
    $elements['table'] = [
      '#type' => 'table',
      '#header' => [$this->t('Year'), $this->t('Publication'), $this->t('Phylogenetic Trees')],
      '#rows' => $rows,
      '#attributes' => [
        'class' => ['carrotomics-table'],
      ],
    ];

    return $elements;
  }

}
