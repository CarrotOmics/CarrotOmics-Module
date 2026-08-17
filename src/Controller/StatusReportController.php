<?php

namespace Drupal\carrotomics\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\tripal_chado\Database\ChadoConnection;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Generates the CarrotOmics Status Report page.
 */

class StatusReportController extends ControllerBase {

  /**
   * The Chado database connection.
   *
   * @var Drupal\tripal_chado\Database\ChadoConnection
   */
  protected ChadoConnection $chado_connection;

  /**
   * Controller constructor.
   */
  public function __construct(ChadoConnection $chado_connection) {
    $this->chado_connection = $chado_connection;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container): static {
    return new static(
      $container->get('tripal_chado.database')
    );
  }

  /**
   * Generates the CarrotOmics Status Report page.
   *
   * @return array
   *   The content render array.
   */
  public function content(): array {

    $elements = [];

    /**
     * Organisms.
     */
    $norganisms = number_format(
      $this->chado_connection->select('1:organism')->countQuery()->execute()->fetchField(),
      0, '', ','
    );
    $nfamilies = number_format(
      $this->chado_connection->query("SELECT COUNT(DISTINCT value) FROM {1:organismprop} WHERE type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='superclass')")->fetchField(),
      0, '', ','
    );
    $ngenera = number_format(
      $this->chado_connection->query("SELECT COUNT(DISTINCT genus) FROM {1:organism} WHERE NOT genus='N/A'")->fetchField(),
      0, '', ','
    );
    $elements['organisms'] = [
      'heading' => [
        '#markup' => '<h3>' . $this->t('Organisms') . '</h3>',
      ],
      'body' => [
        '#markup' => "<p>Currently $norganisms organisms from $ngenera genera in $nfamilies families in the Apiales taxonomic order are present in CarrotOmics.
          They are <a href=\"/data-search/organism\">listed here</a>.</p>
          <p>Both GRIN taxon and NCBI taxon identifier numbers are present as Cross References when available.</p>
          <p>Organism pages list all available germplasm accessions and breeding stocks as well as biomaterial references, unless the number is greater than 100.
          For these organisms, view accessions <a href=\"/data-search/germplasm-accession\">listed here</a></p>",
      ],
    ];

    /**
     * Germplasm and BioSamples.
     */
    $rows = [];
    $results = $this->chado_connection->query("SELECT SC.name, SCP.value, COUNT(*) AS count FROM {1:stockcollection} SC LEFT JOIN {1:stockcollectionprop} SCP ON SC.stockcollection_id=SCP.stockcollection_id LEFT JOIN {1:stockcollection_stock} SCS ON SC.stockcollection_id=SCS.stockcollection_id GROUP BY SC.name, SCP.value ORDER BY SCP.value");
    $acc_total = 0;
    foreach ($results as $result) {
      $abbr = $result->name;
      $name = $result->value;
      $acc_total = $acc_total + $result->count;
      $count = number_format($result->count, 0, '', ',');
      $rows[] = [$count, $name, $abbr];
    }
    $nbio = $this->chado_connection->select('1:biomaterial')->countQuery()->execute()->fetchField();
    $rows[] = [number_format($nbio, 0, '', ','), "\"BioSample\" accessions", ''];
    $acc_total = $acc_total + $nbio;

    $acc_total = number_format($acc_total, 0, '', ',');
    $sp_count = number_format(
      $this->chado_connection->query("SELECT COUNT(DISTINCT organism_id) FROM {stock}")->fetchField(),
      0, '', ',',
    );
    $fam_count = number_format(
      $this->chado_connection->query("SELECT COUNT(DISTINCT P.value) FROM {stock} S LEFT JOIN {organismprop} P ON S.organism_id=P.organism_id WHERE P.type_id=(SELECT cvterm_id FROM {cvterm} WHERE name='superclass')")->fetchField(),
      0, '', ',',
    );

    $elements['germplasm'] = [
      'heading' => [
        '#markup' => '<h3>' . $this->t('Germplasm and BioSamples') . '</h3>',
      ],
      'body' => [
        '#type' => 'table',
        '#header' => ['Number', 'Collection', 'Abbreviation'],
        '#rows' => $rows,
      ],
      'footer' => [
        '#markup' => "<p>These $acc_total germplasm accessions and biosamples represent $sp_count different Apiales species or subspecies from $fam_count families.</p>
          <p>View more information about the collections <a href=\"/data-search/germplasm-collection\">here</a>.</p>
          <p>Geolocation data or origin country is loaded when available.</p>",
      ],
    ];

    return $elements;
  }

}
