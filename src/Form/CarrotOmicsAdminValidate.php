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
use Drupal\tripal\TripalStorage\PluginManager\TripalStorageManager;
use Drupal\tripal_chado\Plugin\TripalStorage\ChadoStorage;
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
   * Tripal storage manager service.
   *
   * @var Drupal\tripal\TripalStorage\PluginManager\TripalStorageManager
   */
  protected TripalStorageManager $tripal_storage_manager;

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
    TripalStorageManager $tripal_storage_manager,
  ) {
    parent::__construct($config_factory, $drupal_connection, $chado_connection, $entity_lookup_manager, $logger, $publish_manager);
    $this->entity_field_manager = $entity_field_manager;
    $this->bundle_info = $bundle_info;
    $this->file_system = $file_system;
    $this->tripal_storage_manager = $tripal_storage_manager;
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
      $container->get('tripal.storage'),
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

    // Add a 'Find double-published records' button and bundle select.
    $form['val_bundle_id'] = [
      '#type'   => 'select',
      '#name' => 'val_bundle_id',
      '#options' => $this->getBundleNames(TRUE),
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

    // Add a 'Validate CarrotOmics accessions' button.
    $form['val_car_acc_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'val_car_acc_btn',
      '#value'  => 'Validate CarrotOmics accessions (UPDATE DISABLED FOR NOW)',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . "For stock items using the CarrotOmics database,"
      . " update the accession in dbxref to match the"
      . " cvterm and CarrotOmics entity e.g. accession/123456"
      . "</em></div><br />",
      '#suffix' => "<hr>",
    ];

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

    return $form;
  }

  /**
   * Gets a list of bundle names.
   *
   * @param bool $include_select
   *   If true, include a "--Select--" item.
   *
   * @return array
   *   The list of bundle names.
   */
  protected function getBundleNames(bool $include_select): array {
    $entity_type = 'tripal_entity';
    $bundles = $this->bundle_info->getBundleInfo($entity_type);
    $select_list = [];
    if ($include_select) {
      $select_list = ['' => '--Select--'];
    }
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
      [$nerrors, $status] = $this->findUnpublished();
    }
    elseif ($triggering_element == 'val_find_doublepublished_btn') {
      [$nerrors, $status] = $this->findDoublepublished($bundle_id);
    }
    elseif ($triggering_element == 'val_car_acc_btn') {
      [$nerrors, $status] = $this->validateCarrotomicsAccessions();
    }
    elseif ($triggering_element == 'val_fileloc_btn') {
      [$nerrors, $status] = $this->validateFileloc();
    }
    elseif ($triggering_element == 'val_eimage_btn') {
      [$nerrors, $status] = $this->validateEimage();
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
   * Find any tripal content types that need to be published.
   *
   * @todo not migrated!
   */
  protected function findUnpublished() {
    $chadoStorage = $this->tripal_storage_manager->createInstance('chado_storage');

    // Key is machine name, value is label.
    $bundles = $this->getBundleNames(FALSE);

    // The output messages will be a list of content types that need to
    // be published, it will be empty if everything is already published.
    $outputmessages = [];

    foreach ($bundles as $bundle_name => $bundle_label) {
      // Retrieve a list of all pkey values for this bundle.
      $record_ids = $chadoStorage->findAllRecordIds($bundle_name) ?? 0;

      // Find a required field for the bundle.
      // Look up entity_ids for the bundle using the drupal field table.
      $published_ids = $this->entity_lookup_manager->getPublishedEntityIds($bundle_name) ?? 0;

      // Find the difference between the arrays.
      $count_diff = count($record_ids) - count($published_ids);

      // Report the results.
      if ($count_diff) {
        $outputmessages[0][] = $bundles[$bundle_name] . ' (' . $bundle_name . ') has '
        . number_format($count_diff) . ' unpublished records out of '
        . number_format(count($record_ids)) . ' total records, '
        . number_format(count($published_ids)) . ' published.';
      }
      else {
        $outputmessages[1][] = $bundles[$bundle_name] . ' (' . $bundle_name . '): ' . number_format(count($record_ids));
      }
    }

    // Put output messages in priority order, unpublished records first.
    $message = '';
    $returncode = 0;
    if ($outputmessages[0] ?? []) {
      $message = '<strong>The following bundles have unpublished records:</strong>';
      $message .= $this->flattenErrors($outputmessages[0]);
      $returncode = count($outputmessages[0]);
    }
    $message .= '<strong>The following bundles are fully published:</strong>';
    $message .= $this->flattenErrors($outputmessages[1] ?? []);

    return [$returncode, $message];
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
      return [1, "Error, no required fields for bundle \"$bundle_id\""];
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
    $outputmessages = [];
    foreach ($items as $record_id => $values) {
      if (count($values) > 1) {
        $n_duplicates++;
        if ($n_duplicates <= 10) {
          $n_summarized++;
          $output_messages[] = 'record_id ' . $record_id . ': entities(' . implode(', ', $values) . ')';
        }
      }
    }
    if ($outputmessages) {
      $subset_label = ($n_summarized == $n_duplicates) ? '' : ', first ' . $n_summarized . ' shown:';
      $first_several = $this->flattenErrors($outputmessages);
      return [$n_duplicates, $n_duplicates . ' duplicates found' . $subset_label . ': ' . $first_several];
    }
    else {
      return [$n_duplicates, 'No duplicates found, ' . number_format(count($items)) . ' records checked.'];
    }
  }

  /**
   * Validates CarrotOmics stock accession numbers.
   *
   * For germplasm accessions using CarrotOmmics as the database,
   * the convention is to have the accession reflect the CarrotOmics
   * type id and entity number, e.g. accession/123456, so that links
   * are clickable. Note that CarrotOmics only uses the germplasm
   * accession content type for all stocks.
   */
  protected function validateCarrotomicsAccessions() {
#\d tripal_entity__germplasm_unique_name
#                           Table "public.tripal_entity__germplasm_unique_name"
#             Column              |          Type          | Collation | Nullable |        Default        
#---------------------------------+------------------------+-----------+----------+-----------------------
# bundle                          | character varying(128) |           | not null | ''::character varying
# deleted                         | smallint               |           | not null | 0
# entity_id                       | bigint                 |           | not null | 
# revision_id                     | bigint                 |           | not null | 
# langcode                        | character varying(32)  |           | not null | ''::character varying
# delta                           | bigint                 |           | not null | 
# germplasm_unique_name_record_id | integer                |           |          | 
# germplasm_unique_name_value     | text                   |           |          | 

    // Find candidate accessions from the CarrotOmics DB.
    // CarrotOmics stores all germplasm types only as "germplasm",
    // so we only need to join a single drupal field table.
    $sql1 = "SELECT S.stock_id, S.dbxref_id, X.accession, T.name, E.entity_id"
    . " FROM {1:stock} S"
    . " LEFT JOIN {1:dbxref} X ON S.dbxref_id=X.dbxref_id"
    . " LEFT JOIN {1:cvterm} T ON S.type_id=T.cvterm_id"
    . " LEFT JOIN {0:tripal_entity__germplasm_unique_name} E ON S.uniquename=E.germplasm_unique_name_value"
    . " WHERE X.db_id=(SELECT db_id FROM {1:db} WHERE name='CarrotOmics')";
    $args1 = [];
dpm($sql1);
    try {
      $results1 = $this->chado_connection->query($sql1, $args1);
    } catch (Exception $e) {
      return [1, $e->getMessage()];
    }

    $ncorrect = 0;
    $nchanged = 0;
    $nunpublished = 0;
    $nerrors = 0;
    $outputmessages = [];

    while ($obj = $results1->fetchObject()) {
      $stock_id = $obj->stock_id;
      $dbxref_id = $obj->dbxref_id;
      $accession = $obj->accession;
      $entity_id = $obj->entity_id;
      // Tripal 3 needed to remove spaces e.g. "generated germplasm".
      // $cvname = preg_replace('/\s/', '', $obj->name);
      // but now everythint iW "germplasm".
      $cvname = 'germplasm';

      // Entity will not be present if not yet published.
      if ($entity_id) {
        $expected = $cvname . '/' . $entity_id;
        if ( $accession == $expected ) {
          $ncorrect++;
        }
        else {
          // Incorrect accession, need to update.
          $link = '<a class="carrotomics-admin-link-button" href="/bio_data/' . $entity_id . '">' . $stock_id . '</a> ⟶ ';
          $outputmessages[] = $link . "Update accession from \"$accession\" to \"$expected\"";
          $sql3 = "UPDATE {1:dbxref} SET accession=:acc WHERE dbxref_id=:dbxref_id";
          $args3 = [ ':acc' => $expected, ':dbxref_id' => $dbxref_id ];
          try {
#@todo don't change yet            $results3 = $this->chado_connection->query($sql3, $args3);
          } catch (Exception $e) {
            return [1, $e->getMessage()];
          }
          $nchanged++;
        }
      }
      else {
        $nunpublished++;
      }
    }

    return [$nerrors, "$nchanged accessions updated, $ncorrect were correct, $nunpublished need to be published " . $this->flattenErrors($outputmessages)];
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
      if ($nerrors >= 100) {
        $outputmessages[] = 'Too many errors, further errors are not shown';
        break;
      }
    }

    if ($outputmessages) {
      return [1, "$nerrors errors found" . $this->flattenErrors($outputmessages)];
    }
    else {
      return [0, "$nchecked public:// files checked, there were no problems found"];
    }
  }

  /**
   * Validate urls in the eimage table.
   *
   * Ensure that all URLs in the chado.eimage table point to an existing file.
   */
  protected function validateEimage() {
    // Ihe output message will be a list of problems found.
    // It will be empty if everything is correct.
    $outputmessage = '';
    $nchecked = 0;
    $nerrors = 0;

    // Regex pattern and replacements are defined in the formatter settings.
    // We could assume that all table entries are for germplasm accessions.
    // That may not be true in the future. We will instead assume that
    // CarrotOmics always uses the same thumbnail regex configuration.
    $regex_pattern = 'eimage\/image';
    $regex_replacement = 'eimage/thumbnail';

    // Get a list of files to check, order by eimage_id for a somewhat
    // chronological list, then check each one.
    $sql = "SELECT eimage_id, image_uri FROM {1:eimage} ORDER BY eimage_id";
    $results = $this->chado_connection->query($sql, []);
    while ($obj = $results->fetchObject()) {
      $eimage_id = $obj->eimage_id;
      $image_uri = $obj->image_uri;
      $nchecked++;

      // Convert to filesystem full path.
      // For CarrotOmics all images are locally hosted under public://.
      $full_image_path = $this->file_system->realpath($image_uri);
      $thumbnail_path = preg_replace('/' . $regex_pattern . '/', $regex_replacement, $full_image_path);

      // 1. validate full resolution image existence
      if (!file_exists($full_image_path)) {
        $outputmessage .= "<li>Image $eimage_id not present \"$image_uri\"</li>";
        $nerrors++;
      }
      // 2. validate full resolution image size
      elseif (!filesize($full_image_path)) {
        $outputmessage .= "<li>Zero size image file $eimage_id \"$image_uri\"</li>";
        $nerrors++;
      }
      // 3. validate thumbnail image existence
      if (!file_exists($thumbnail_path)) {
        $outputmessage .= "<li>Thumbnail $eimage_id not present \"$thumbnail_path\"</li>";
        $nerrors++;
      }
      // 4. validate thumbnail image size
      elseif (!filesize($thumbnail_path)) {
        $outputmessage .= "<li>Zero size thumbnail image file $eimage_id \"$thumbnail_path\"</li>";
        $nerrors++;
      }
      if ($nerrors >= 100) {
        $outputmessage .= "<li value=\"0\">Too many errors, further errors are not shown</li>";
        break;
      }
    }

    if ($outputmessage) {
      return [1, "$nerrors errors found<ol>$outputmessage</ol>"];
    }
    else {
      return [0, "$nchecked files checked, there were no problems found"];
    }
  }

}
