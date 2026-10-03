<?php

namespace Drupal\carrotomics\Plugin\TripalBlastLinkout;

use Drupal\Component\Plugin\PluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Link;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\Core\Url;
use Drupal\pgsql\Driver\Database\pgsql\Connection;
use Drupal\tripal\Services\TripalLogger;
use Drupal\tripal_blast\Attribute\TripalBlastLinkout;
use Drupal\tripal_blast\Plugin\TripalBlastLinkout\TripalBlastLinkoutInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * This linkout type creates a link to a gene entity page.
 */
#[TripalBlastLinkout(
  id: 'CarrotOmicsGene',
  label: new TranslatableMarkup('CarrotOmics Gene Link'),
  description: new TranslatableMarkup('Provides a link to a gene entity page'),
  weight: 8,
)]
class CarrotOmicsLinkoutGene extends PluginBase implements TripalBlastLinkoutInterface, ContainerFactoryPluginInterface {

  /**
   * A database connection to the drupal public schema.
   *
   * @var Drupal\pgsql\Driver\Database\pgsql\Connection
   */
  protected Connection $drupal_connection;

  /**
   * The Tripal logger service.
   *
   * @var Drupal\tripal\Services\TripalLogger
   */
  protected TripalLogger $tripal_logger;

  /**
   * Constructs a TripalBlastLinkoutLink object.
   */
  public function __construct(
    array $configuration,
    $plugin_id,
    $plugin_definition,
    Connection $drupal_connection,
    TripalLogger $tripal_logger,
  ) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->drupal_connection = $drupal_connection;
    $this->tripal_logger = $tripal_logger;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('database'),
      $container->get('tripal.logger'),
    );
  }

  /**
   * {@inheritDoc}
   */
  public function createLink(\SimpleXMLElement $hit): Link|string {
    // Fallback if link cannot be created is just the hit name.
    $link = $hit->hit_name;

    // For this link method, url prefix is to a gene entity, so should be
    // "base://gene/". ID is the gene name, e.g. "DCAR_000101".
    if (!isset($hit->url_prefix) || !isset($hit->linkout_id)) {
      return $link;
    }

    // Query a drupal field table to find the entity ID.
    $field_table = 'tripal_entity__gene_name';
    $query = $this->drupal_connection->select($field_table, 'E');
    $query->condition('E.gene_name_value', $hit->linkout_id, '=');
    $query->addField('E', 'entity_id', 'entity_id');

    // We can only have zero or one hits to the uniquename field.
    $result = $query->execute()->fetchField();
    if ($result) {
      try {
        $url = Url::fromUri($hit->url_prefix . $result);
        $url->setOptions([
          'attributes' => [
            'target' => '_blank',
          ],
        ]);
        $link = Link::fromTextAndUrl($hit->linkout_id, $url);
      }
      catch (\Exception $e) {
        // A failed link situation should not be shown to end-users, because
        // they can't do anything about it, but the site admin should see it,
        // so we need to just log this.
        $this->tripal_logger->error('TripalBlastLinkout "' . $this->getPluginId() . '" error: ' . $e->getMessage());
      }
    }

    return $link;
  }

}
