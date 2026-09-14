<?php

namespace Drupal\carrotomics\Form;

use Drupal\Core\Config\ConfigFactory;
use Drupal\Core\Entity\EntityFieldManager;
use Drupal\Core\Entity\EntityTypeBundleInfo;
use Drupal\Core\File\FileSystem;
use Drupal\Core\Form\FormStateInterface;
use Drupal\pgsql\Driver\Database\pgsql\Connection;
use Drupal\tripal_chado\Database\ChadoConnection;
use Drupal\tripal\Services\TripalEntityLookup;
use Drupal\tripal\Services\TripalLogger;
use Drupal\tripal\TripalBackendPublish\PluginManager\TripalBackendPublishManager;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides the user interface for CarrotOmics validation tools.
 */
class CarrotOmicsAdminValidate extends CarrotOmicsAdminFormBase {

  /**
   * Entity field manager service.
   *
   * @var Drupal\Core\Entity\EntityFieldManager
   */
  protected EntityFieldManager $entity_field_manager;

  /**
   * Bundle info service.
   *
   * @var Drupal\Core\Entity\EntityTypeBundleInfo
   */
  protected EntityTypeBundleInfo $bundle_info;

  /**
   * Drupal configuration service.
   *
   * @var Drupal\Core\File\FileSystem
   */
  protected FileSystem $file_system;

  /**
   * CarrotOmicsAdminValidate class constructor.
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
    EntityFieldManager $entity_field_manager,
    EntityTypeBundleInfo $bundle_info,
    FileSystem $file_system,
  ) {
    parent::__construct($config_factory, $drupal_connection, $chado_connection, $entity_lookup_manager, $logger, $publish_manager);
    $this->entity_field_manager = $entity_field_manager;
    $this->bundle_info = $bundle_info;
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
      $container->get('entity_field.manager'),
      $container->get('entity_type.bundle.info'),
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
    return 'carrotomics_admin_validate_form';
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
        '#type' => 'fieldset',
        '#title' => 'Results',
        '#markup' => '<div class="form-results">' . $storage['results'] . '</div>',
        '#weight' => -100,
      ];
    }

    // ;;;
    if (FALSE) {
      // Add a 'Find Unpublished Content' button.
      $form['val_find_unpub_btn'] = [
        '#type'   => 'submit',
        '#name'   => 'val_find_unpub_btn',
        '#value'  => 'Find Unpublished Content',
        '#prefix' => '<div style="padding-top:30px;"><em>'
        . "List any Tripal content types that have unpublished content"
        . "</em></div><br />",
        '#suffix' => "<hr>",
      ];
    } //;;;

    // Add a 'Find double-published records' button and bundle select.
    $form['val_bundle_id'] = [
      '#type'   => 'select',
      '#name' => 'val_bundle_id',
      '#options' => $this->getBundleNames(),
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . 'Finds chado records that have been published as more than one entity.'
      . '</em></div><br />',
    ];

    $form['val_find_doublepublished_btn'] = [
      '#type' => 'submit',
      '#name' => 'val_find_doublepublished_btn',
      '#value' => 'Find double-published records',
      '#suffix' => '<hr>',
    ];

    if (FALSE) {
      // Add a 'Validate CarrotOmics accessions' button.
      $form['val_car_acc_btn'] = [
        '#type'   => 'submit',
        '#name'   => 'val_car_acc_btn',
        '#value'  => 'Validate CarrotOmics accessions',
        '#prefix' => '<div style="padding-top:30px;"><em>'
        . "For stock items using the CarrotOmics database,"
        . " update the accession in dbxref to match the"
        . " cvterm and CarrotOmics entity e.g. accession/123456"
        . "</em></div><br />",
        '#suffix' => "<hr>",
      ];

    } // ;;;
    // Add a 'Validate public:// fileloc entries' button.
    $form['val_fileloc_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'val_fileloc_btn',
      '#value'  => 'Validate public:// chado.fileloc entries',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . 'Validate all files stored locally using the public:// URL'
      . ' nomenclature that are listed in the chado.fileloc table.'
      . ' File presence, read permission, size, and MD5 checksum are validated.'
      . '</em></div><br />',
      '#suffix' => '<hr>',
    ];

    // ;;;
    if (FALSE) {
      // Add a 'Validate eimage urls' button.
      $form['val_eimage_btn'] = [
        '#type'   => 'submit',
        '#name'   => 'val_eimage_btn',
        '#value'  => 'Validate eimage urls',
        '#prefix' => '<div style="padding-top:30px;"><em>'
        . "Validate all eimage urls to verify file is present"
        . "</em></div><br />",
        '#suffix' => "<hr>",
      ];

    } // ;;;
    return $form;
  }

  /**
   * Gets a list of bundle names.
   *
   * @return array
   *   The list of bundle names.
   */
  protected function getBundleNames(): array {
    $entity_type = 'tripal_entity';
    $bundles = $this->bundle_info->getBundleInfo($entity_type);
    $select_list = ['' => '--Select--'];
    // $select_list = ['all' => '--All Bundles--'];
    foreach ($bundles as $bundle_id => $info) {
      $select_list[$bundle_id] = $info['label'];
    }
    return $select_list;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {

    $form_state->setRebuild(TRUE);

    // Gets the triggering element, i.e. which button was pressed.
    $triggering_element = $form_state->getTriggeringElement()['#name'];

    // Gets the values from form input fields.
    $bundle_id = $form_state->getValue('val_bundle_id');

    // Take action depending on which button was pressed.
    $nerrors = 0;
    $status = '';
    if ($triggering_element == 'val_find_unpub_btn') {
      [$nerrors, $status] = carrotomics_admin_find_unpublished();
    }
    elseif ($triggering_element == 'val_find_doublepublished_btn') {
      [$nerrors, $status] = $this->findDoublepublished($bundle_id);
    }
    elseif ($triggering_element == 'val_car_acc_btn') {
      [$nerrors, $status] = carrotomics_admin_validate_carrotomics_accessions();
    }
    elseif ($triggering_element == 'val_fileloc_btn') {
      [$nerrors, $status] = $this->validateFileloc();
    }
    elseif ($triggering_element == 'val_eimage_btn') {
      [$nerrors, $status] = carrotomics_admin_validate_eimage();
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
   * Find and unpublished chado records published more than once.
   *
   * This probably happened because of some bug.
   *
   * @param string $bundle_id
   *   The name of the bundle to check.
   */
  protected function findDoublepublished(string $bundle_id) {
    if (!$bundle_id) {
      $this->messenger()->addError("Bundle not selected");
      return;
    }

    // Get a required field for this bundle, plus the base table.
    $base_table = NULL;
    $required_field = NULL;
    $fields = $this->entity_field_manager->getFieldDefinitions('tripal_entity', $bundle_id);
    foreach ($fields as $field_name => $field_info) {
      $storage_settings = $field_info->getSetting('storage_plugin_settings');
      $base_table = $storage_settings['base_table'] ?? '';
      $base_column = $storage_settings['base_table_dependant']['base_column']
        ?? $storage_settings['base_column'] ?? '';
      $is_required = $field_info->isRequired();
      if ($is_required and $base_table and $base_column) {
        $required_field = $field_name;
        break;
      }
    }
    if (!$required_field) {
      $this->messenger()->addError("No required fields for bundle \"$bundle_id\"");
      return;
    }

    // Set up a drupal field table query.
    $table_name = 'tripal_entity__' . $required_field;
    $record_id_column = $required_field . '_record_id';
    $query = $this->drupal_connection->select($table_name, 'T');
    $query->addField('T', 'entity_id', 'entity_id');
    $query->addField('T', $record_id_column, 'record_id');
    $query->condition('T.bundle', $bundle_id, '=');
    $query->orderBy('entity_id');
    $results = $query->execute();

    // Parse query results.
    $items = [];
    while ($result = $results->fetchObject()) {
      $items[$result->record_id][] = $result->entity_id;
    }

    // Count duplicates.
    $n_duplicates = 0;
    $n_summarized = 0;
    $first_several = '';
    foreach ($items as $record_id => $values) {
      if (count($values) > 1) {
        $n_duplicates++;
        if ($n_duplicates <= 10) {
          $n_summarized++;
          $first_several .= '<br>record_id ' . $record_id . ': entities(' . implode(', ', $values) . ')';
        }
      }
    }

    $subset_label = ($n_summarized == $n_duplicates) ? '' : ' First ' . $n_summarized;
    return [$n_duplicates, $n_duplicates . ' duplicates found' . $subset_label . ': ' . $first_several];
  }

  /**
   * Validate files used by the tripal file module.
   *
   * Ensures that all URLs in the chado.fileloc table using the public://
   * method of access are present. Checksums and file sizes are checked
   * for correctness.
   */
  protected function validateFileloc() {
    // Ihe output messages are a list of problems found.
    // It will be empty if everything is correct.
    $outputmessages = [];
    $nchecked = 0;
    $nerrors = 0;

    // Get the base path of where public:// points.
    $files_path = $this->file_system->realpath('public://');

    // Get a list of files to check, order by file_id for a somewhat
    // chronological list.
    $query = $this->chado_connection->select('1:fileloc', 'L');
    $query->fields('L', ['file_id', 'uri', 'md5checksum', 'size']);
    $query->orderBy('file_id', 'ASC');
    $results = $query->execute();

    // Main loop to check files.
    foreach ($results as $obj) {
      $uri = $obj->uri;
      // Only validate local files using public:// prefix.
      if (preg_match('|^public://|', $uri)) {
        $nchecked++;
        $file_id = $obj->file_id;
        $md5checksum = $obj->md5checksum;
        $size = $obj->size;
        $real_path = $files_path . preg_replace('|^public://|', '/', $uri);
        // 1. Validate existence.
        if (!file_exists($real_path)) {
          $outputmessages[] = $this->entityLink('file', $file_id) . "File not present \"$uri\"";
          $nerrors++;
        }
        // 2. Validate file permissions.
        elseif (!is_readable($real_path)) {
          $outputmessages[] = $this->entityLink('file', $file_id) . "File permission problem for \"$uri\", file is not readable";
          $nerrors++;
        }
        else {
          // 3. Validate size.
          $actualsize = filesize($real_path);
          if ($size != $actualsize) {
            $outputmessages[] = $this->entityLink('file', $file_id) . "Size stored for file \"$uri\" $size is different than actual size $actualsize";
            $nerrors++;
          }
          // 4. Warn if no md5 checksum. When missing in chado.fileloc,
          // it is still defined as a string of 32 space characters.
          elseif ((!$md5checksum) or (preg_match('/ /', $md5checksum))) {
            $outputmessages[] = $this->entityLink('file', $file_id) . "There is no valid MD5 checksum stored for file \"$uri\"";
            $nerrors++;
          }
          else {
            // 5. Validate md5 checksum.
            try {
              $actualmd5checksum = md5_file($real_path);
              if ($md5checksum != $actualmd5checksum) {
                $outputmessages[] = $this->entityLink('file', $file_id) . "MD5 checksum \"$md5checksum\" stored for file \"$uri\" is different than actual checksum \"$actualmd5checksum\"";
                $nerrors++;
              }
            }
            catch (Exception $e) {
              $outputmessages[] = $this->entityLink('file', $file_id) . "File read error for file \"$uri\" " . $e->getMessage();
              $nerrors++;
            }
          }
        }
      }
    }

    if ($outputmessages) {
      return [1, "$nerrors errors found" . $this->flattenErrors($outputmessages)];
    }
    else {
      return [0, "$nchecked public:// files checked, there were no problems found"];
    }
  }

}
