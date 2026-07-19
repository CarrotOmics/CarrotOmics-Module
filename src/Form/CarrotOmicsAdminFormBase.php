<?php

namespace Drupal\carrotomics\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\pgsql\Driver\Database\pgsql\Connection;
use Drupal\tripal_chado\Database\ChadoConnection;
use Drupal\tripal\Services\TripalEntityLookup;
use Drupal\tripal\Services\TripalLogger;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\BinaryFileResponse;
use Symfony\Component\HttpFoundation\ResponseHeaderBag;

/**
 * Provides the user interface for CarrotOmics publication tools.
 */
abstract class CarrotOmicsAdminFormBase extends FormBase {

  /**
   * A database connection to the drupal public schema.
   *
   * @var Drupal\pgsql\Driver\Database\pgsql\Connection
   */
  protected Connection $drupal_connection;

  /**
   * A TripalDBX connection to chado.
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
   * The TripalLogger for logging messages.
   *
   * @var Drupal\tripal\Services\TripalLogger
   */
  protected TripalLogger $logger;

  /**
   * A temporary file used when returning data.
   *
   * @var string
   */
  protected string $result_file_path = 'temporary://carrotomics_admin.tsv';

  /**
   * Stores cvterm_id values.
   *
   * @var array
   */
  protected array $cvterms;

  /**
   * A list of publications that we have updated.
   *
   * Use the array keys to store the pub_id values to republish to
   * eliminate the possibility of duplicates. Values are ignored.
   *
   * @var array
   */
  protected array $needs_republishing = [];

  /**
   * CarrotOmicsAdminFormBase class constructor.
   *
   * Prepares injected services.
   */
  public function __construct(
    Connection $drupal_connection,
    ChadoConnection $chado_connection,
    TripalEntityLookup $entity_lookup_manager,
    TripalLogger $logger,
  ) {
    $this->drupal_connection = $drupal_connection;
    $this->chado_connection = $chado_connection;
    $this->entity_lookup_manager = $entity_lookup_manager;
    $this->logger = $logger;
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
    );
  }

  /**
   * Converts an array into markup of an ordered list.
   *
   * @param array $errors
   *   A collection of zero or more error messages.
   *
   * @return string
   *   An empty string if the array was empty, otherwise an ordered
   *   list of HTML markup.
   */
  protected function flattenErrors(array $errors): string {
    $flaterrors = '';
    if ($errors) {
      $flaterrors = '<ol><li>';
      $flaterrors .= implode('</li><li>', $errors);
      $flaterrors .= '</li></ol>';
    }
    return $flaterrors;
  }

  /**
   * Looks up a cvterm_id value from its name and optional CV.
   *
   * @string $term_name
   *   The name of the term to look up.
   * @string|null $cv_name
   *   Optional controlled vocabulary name, defaults to 'tripal_pub'.
   *
   * @return int
   *   The cvterm_id value.
   */
  protected function lookupCvterm(string $term_name, ?string $cv_name = 'tripal_pub'): int {
    $key = $cv_name . ':' . $term_name;
    if (isset($this->cvterms[$key])) {
      return $this->cvterms[$key];
    }
    else {
      $query = $this->chado_connection->select('1:cvterm', 'T');
      $query->fields('T', ['cvterm_id']);

      // Subquery: Get cv_id from cv table.
      $subquery_cv = $this->chado_connection->select('1:cv', 'V')
        ->fields('V', ['cv_id'])
        ->condition('name', $cv_name, '=');

      // Add a WHERE clause using the subquery.
      $query->condition('T.cv_id', $subquery_cv, '=');
      $query->condition('T.name', $term_name, '=');

      $results = $query->execute()->fetchAll();
      if (count($results) != 1) {
        throw new \Exception("Failed looking up CV term $key");
      }
      $this->cvterms[$key] = $results[0]->cvterm_id;
      return $this->cvterms[$key];
    }
  }

  /**
   * Generates a link to an entity styled as a button.
   *
   * @param string $bundle
   *   The bundle ID, e.g. 'pub'.
   * @param int $record_id
   *   A pkey value for a chado table.
   * @param bool|null $add_arrow
   *   If TRUE (default), then include a right arrow.
   *
   * @return string
   *   HTML for the button.
   */
  protected function entityLink(string $bundle, int $record_id, ?bool $add_arrow = TRUE): string {
    $entity_id = $this->entity_lookup_manager->getEntityId($record_id, NULL, NULL, $bundle);
    $link = '<a class="carrotomics-admin-link-button" href="/bio_data/' . $entity_id . '">' . $record_id . '</a>';
    if ($add_arrow) {
      $link .= ' ⟶ ';
    }
    return $link;
  }

  /**
   * Republishes any updated records.
   *
   * The list of records to republish is stored as the array keys in the
   * class variable $this->needs_republishing.
   *
   * @param string $bundle
   *   The name of the bundle to republish, defaults to "pub".
   * @param string|null $datastore
   *   The name of the bundle to republish, defaults to "chado_storage".
   *
   * @return void
   *   No return value.
   */
  protected function republish(string $bundle, string $datastore = 'chado_storage'): void {
    // @todo if there are many records, launch as a job.
    if ($this->needs_republishing) {
      $publish_instance = $this->publish_manager->createInstance($datastore);
      $schema_name = $this->config_factory->get('tripal_chado.settings')->get('default_schema');
      $publish_options = [
        'record_ids' => array_keys($this->needs_republishing),
        'schema_name' => $schema_name,
        'batch_size' => 100,
        'republish' => TRUE,
        'migration-file' => NULL,
        'lenient-migration' => NULL,
        'bundle' => $bundle,
        'datastore' => $datastore,
        'job' => NULL,
      ];
      $publish_instance->publish($publish_options);
      $this->needs_republishing = [];
    }
  }

  /**
   * Returns a file after a form submit.
   *
   * @var string $label
   *   The label for the file, e.g. 'Useful Data'.
   * @var Drupal\Core\Form\FormStateInterface $form_state
   *   The form state.
   *
   * @return void
   *   No return value.
   */
  protected function returnFile(string $label, FormStateInterface $form_state) {
    // Create the BinaryFileResponse.
    $response = new BinaryFileResponse($this->result_file_path);

    // Set the Content-Disposition header so the browser knows to download it.
    $response->setContentDisposition(ResponseHeaderBag::DISPOSITION_ATTACHMENT, $label);

    // Set the response in the form state instead of a standard redirect.
    $form_state->setResponse($response);
  }

}
