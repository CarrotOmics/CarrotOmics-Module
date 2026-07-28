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
class CarrotOmicsAdminValidate extends CarrotOmicsAdminFormBase {

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
        '#type'   => 'fieldset',
        '#title' => 'Results',
        '#markup' => '<div class="form-results">' . $storage['results'] . '</div>',
        '#weight' => -100,
      ];
    }

    // ;;;
    if (FALSE) {
      // Add a 'Find Unpublished Content' button.
      $form['find_unpub_btn'] = [
        '#type'   => 'submit',
        '#name'   => 'find_unpub_btn',
        '#value'  => 'Find Unpublished Content',
        '#prefix' => '<div style="padding-top:30px;"><em>'
        . "List any Tripal content types that have unpublished content"
        . "</em></div><br />",
        '#suffix' => "<hr>",
      ];

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
      . "Validate all files stored locally using the public:// URL"
      . " nomenclature that are listed in the chado.fileloc table."
      . " File presence, read permission, size, and MD5 checksum are validated."
      . "</em></div><br />",
      '#suffix' => "<hr>",
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
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {

    $form_state->setRebuild(TRUE);

    // Gets the triggering element, i.e. which button was pressed.
    $triggering_element = $form_state->getTriggeringElement()['#name'];

    // Take action depending on which button was pressed.
    $nerrors = 0;
    $status = '';
    if ($triggering_element == 'find_unpub_btn') {
      [$nerrors, $status] = carrotomics_admin_find_unpublished();
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
