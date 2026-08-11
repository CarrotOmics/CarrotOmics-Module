<?php

namespace Drupal\carrotomics\Plugin\Field\FieldType;

use Drupal\core\Field\FieldDefinitionInterface;
use Drupal\core\Form\FormStateInterface;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\tripal\TripalField\Attribute\TripalFieldType;
use Drupal\tripal\Entity\TripalEntityType;
use Drupal\tripal_chado\TripalField\ChadoFieldItemBase;
use Drupal\tripal_chado\TripalStorage\ChadoIntStoragePropertyType;
use Drupal\tripal_chado\TripalStorage\ChadoRealStoragePropertyType;
use Drupal\tripal_chado\TripalStorage\ChadoTextStoragePropertyType;
use Drupal\tripal_chado\TripalStorage\ChadoVarCharStoragePropertyType;

/**
 * Plugin implementation of the 'chado_nd_geolocation' field type.
 *
 * Note that this field is a triple-hop field!
 * e.g. stock -> nd_experiment_stock -> nd_experiment -> nd_geolocation.
 */
#[TripalFieldType(
  id: 'chado_nd_geolocation',
  category: 'tripal_chado',
  label: new TranslatableMarkup('Chado Geographic Position'),
  description: new TranslatableMarkup('Geographic position coordinates: latitude, longitude, and altitude.'),
  default_widget: 'chado_nd_geolocation_widget',
  default_formatter: 'chado_nd_geolocation_formatter',
)]
class ChadoNdGeolocationTypeItem extends ChadoFieldItemBase {

  /**
   * The unique identifier of this field.
   *
   * Matches the id in the Attribute.
   *
   * @var string
   */
  public static $id = "chado_nd_geolocation";

 /**
   * The chado table which is the object of the relationship.
   *
   * Because this is an unusual triple-hop field, this table is just the
   * second hop, the final table will be nd_geolocation and is hard-coded.
   *
   * @var string
   */
  protected static $object_table = 'nd_experiment';

  /**
   * The foreign key that links the linking table to the object table.
   *
   * Note: this should be in all fields linking a base table to another
   * main chado table (i.e. object table).
   *
   * @var string
   */
  protected static $object_id = 'nd_experiment_id';

  /**
   * {@inheritdoc}
   */
  public static function mainPropertyName() {
    // The property that indicates if this field is empty.
    return 'nd_geo_lat'; //;;;self::$object_id;
  }

  /**
   * {@inheritdoc}
   */
  public static function mainDisplayPropertyName() {
    // The property to use in the entity title/url.
    return 'nd_geo_lat';  // @todo change to description?
  }

  /**
   * Flag to indicate a base column selector should be shown.
   *
   * @var bool
   *   When TRUE the ChadoFieldItemBase parent class will add a base column
   *   select list on the field configuration form. When FALSE, no selector
   *   will be available.
   */
  protected static $select_base_column = FALSE;

  /**
   * Valid column types to pass to the ChadoFieldItemBase parent class.
   *
   * @var array
   *   Columns of the types listed here will be added to the select list on
   *   the field configuration form.
   */
  protected static $valid_base_column_types = [];

  /**
   * {@inheritdoc}
   */
  public static function defaultFieldSettings() {
    $field_settings = parent::defaultFieldSettings();
    // CV Term is 'geographic position';
    $field_settings['termIdSpace'] = 'SIO';
    $field_settings['termAccession'] = '000013';
    return $field_settings;
  }

  /**
   * {@inheritdoc}
   */
  public static function defaultStorageSettings() {
    $storage_settings = parent::defaultStorageSettings();
    $storage_settings['storage_plugin_settings']['linking_method'] = '';
    $storage_settings['storage_plugin_settings']['linker_table'] = '';
    $storage_settings['storage_plugin_settings']['linker_fkey_column'] = '';
    $storage_settings['storage_plugin_settings']['object_table'] = self::$object_table;
    return $storage_settings;
  }

  /**
   * {@inheritdoc}
   */
  public function storageSettingsForm(array &$form, FormStateInterface $form_state, $has_data) {
    $elements = parent::storageSettingsForm($form, $form_state, $has_data);
    return $elements;
  }

  /**
   * {@inheritdoc}
   */
  public static function tripalTypes($field_definition) {

    // Retrieve the storage settings.
    $storage_settings = $field_definition->getSetting('storage_plugin_settings');
    $base_table = $storage_settings['base_table'];

    // If we don't have a base table then we're not ready to specify the
    // properties for this field.
    if (!$base_table) {
      return;
    }

    // Get the various tables and columns needed for this field.
    // We will get the terms by using the Chado table columns they map to.
    $chado = \Drupal::service('tripal_chado.database');
    $schema = $chado->schema();
    $entity_type_id = $field_definition->getTargetEntityTypeId();

    // Base table.
    $base_pkey_col = self::getPrimaryKey($base_table, $schema);

    // Object table.
    $object_table = self::$object_table;
    $object_schema_def = self::getChadoTableDef($object_table, $schema);
    $object_pkey_col = $object_schema_def['primary key'];

    // Get the property terms by using the Chado table columns they map to.
    $object_table = self::$object_table;
    $object_schema_def = self::getChadoTableDef($object_table, $schema);

    // Standard linker table.
    [$linker_table, $linker_fkey_column] = self::get_linker_table_and_column($storage_settings, $base_table, $object_pkey_col);
    $linker_schema_def = self::getChadoTableDef($linker_table, $schema);
    $linker_pkey_col = $linker_schema_def['primary key'];
    $linker_left_col = self::getChadoForeignKeyColumn($linker_table, $base_table, $schema);
    $linker_left_term = self::getColumnTermId($linker_table, $linker_left_col, self::$record_id_term);
    $linker_fkey_term = self::getColumnTermId($linker_table, $linker_fkey_column, self::$record_id_term);
    // Only some linker tables have a type_id column.
    $linker_type_id_term = NULL;
    if (array_key_exists('type_id', $linker_schema_def['fields'])) {
      $linker_type_id_term = self::getColumnTermId($linker_table, 'type_id', 'schema:additionalType');
    }

    // Second level linker table.
    $nd_exp_type_id_term = self::getColumnTermId('nd_experiment', 'type_id', 'schema:additionalType');

    // Cvterm table, to retrieve the cvterm name for the linker
    // and nd_experiment types.
    $cvterm_schema_def = self::getChadoTableDef('cvterm', $schema);
    $cvterm_name_term = self::getColumnTermId('cvterm', 'name', 'schema:name');
    $cvterm_name_len = $cvterm_schema_def['fields']['name']['size'];

    // The final table, nd_geolocation, is specific to this field,
    // so it is hard-coded.
    $nd_geolocation_schema_def = self::getChadoTableDef('nd_geolocation', $schema);
    $latitude_term = self::getColumnTermId('nd_geolocation', 'latitude', 'SIO:000319');
    $longitude_term = self::getColumnTermId('nd_geolocation', 'longitude', 'SIO:000318');
    $altitude_term = self::getColumnTermId('nd_geolocation', 'altitude', 'SIO:000438');
    $geodetic_datum_term = self::getColumnTermId('nd_geolocation', 'geodetic_datum', 'GEO:000000788');
    $geodetic_datum_len = $nd_geolocation_schema_def['fields']['geodetic_datum']['size'];
    $description_term = self::getColumnTermId('nd_geolocation', 'description', 'schema:description');

    $properties = [];

    // Define the base table record id.
    $properties[] = new ChadoIntStoragePropertyType($entity_type_id, self::$id, 'record_id', self::$record_id_term, [
      'action' => 'store_id',
      'drupal_store' => TRUE,
      'path' => $base_table . '.' . $base_pkey_col,
    ]);

    // This property will store the Drupal entity ID of the linked chado
    // record, if one exists.
    $properties[] = new ChadoIntStoragePropertyType($entity_type_id, self::$id, 'entity_id', self::$drupal_entity_term, [
      'action' => 'function',
      'drupal_store' => TRUE,
      'namespace' => self::$chadostorage_namespace,
      'function' => self::$drupal_entity_callback,
      'ftable' => self::$object_table,
      'fkey' => $linker_fkey_column,
    ]);

    // Define the linker table that links the base table to the object table.
    $properties[] = new ChadoIntStoragePropertyType($entity_type_id, self::$id, 'linker_id', self::$record_id_term, [
      'action' => 'store_pkey',
      'drupal_store' => TRUE,
      'path' => $linker_table . '.' . $linker_pkey_col,
    ]);

    // Define the link between the base table and the linker table.
    $properties[] = new ChadoIntStoragePropertyType($entity_type_id, self::$id, 'link', $linker_left_term, [
      'action' => 'store_link',
      'drupal_store' => FALSE,
      'path' => $base_table . '.' . $base_pkey_col . '>' . $linker_table . '.' . $linker_left_col,
    ]);

    // Define the link between the linker table and the object
    // table, i.e. nd_experiment.
    $properties[] = new ChadoIntStoragePropertyType($entity_type_id, self::$id, self::$object_id, $linker_fkey_term, [
      'action' => 'store',
      'drupal_store' => TRUE,
      'path' => $linker_table . '.' . $linker_fkey_column,
      'delete_if_empty' => TRUE,
      'empty_value' => 0,
    ]);

    // Only some linker tables have a type_id column.
    if ($linker_type_id_term) {
      $properties[] = new ChadoIntStoragePropertyType($entity_type_id, self::$id, 'linker_type_id', $linker_type_id_term, [
        'action' => 'store',
        'drupal_store' => FALSE,
        'path' => $linker_table . '.type_id',
        'as' => 'linker_type_id',
      ]);
      $properties[] = new ChadoVarCharStoragePropertyType($entity_type_id, self::$id, 'linker_type', $linker_type_id_term, $cvterm_name_len, [
        'action' => 'read_value',
        'drupal_store' => FALSE,
        'path' => $linker_table . '.type_id>cvterm.cvterm_id;name',
        'as' => 'linker_type',
      ]);
    }

    // The nd_experiment table has a type_id.
    $properties[] = new ChadoIntStoragePropertyType($entity_type_id, self::$id, 'nd_exp_type_id', self::$record_id_term, [
      'action' => 'store',
      'drupal_store' => FALSE,
      'path' => $linker_table . '.' . $linker_fkey_column . '>nd_experiment.nd_experiment_id;type_id',
      'as' => 'nd_exp_type_id',
    ]);
    $properties[] = new ChadoVarCharStoragePropertyType($entity_type_id, self::$id, 'nd_exp_type', $nd_exp_type_id_term, $cvterm_name_len, [
      'action' => 'read_value',
      'drupal_store' => FALSE,
      'path' => $linker_table . '.' . $linker_fkey_column . '>nd_experiment.nd_experiment_id;nd_experiment.type_id>cvterm.cvterm_id;name',
      'as' => 'nd_exp_type',
    ]);

    // The remaining properties are from the final (third) hop to
    // the nd_geolocation table. Because this table is linked to from the
    // object table (nd_experiment), and this constitutes an additional
    // hop, we cannot use a 'store' action here because the needed join
    // will not be available.
    $properties[] = new ChadoRealStoragePropertyType($entity_type_id, self::$id, 'nd_geo_lat', $latitude_term, [
      'action' => 'read_value',
      'drupal_store' => FALSE,
      'path' => $linker_table . '.' . $linker_fkey_column . '>' . $object_table . '.' . $object_pkey_col
      . ';' . $object_table . '.nd_geolocation_id>nd_geolocation.nd_geolocation_id;latitude',
      'as' => 'nd_geo_lat',
    ]);
    $properties[] = new ChadoRealStoragePropertyType($entity_type_id, self::$id, 'nd_geo_lon', $longitude_term, [
      'action' => 'read_value',
      'drupal_store' => FALSE,
      'path' => $linker_table . '.' . $linker_fkey_column . '>' . $object_table . '.' . $object_pkey_col
      . ';' . $object_table . '.nd_geolocation_id>nd_geolocation.nd_geolocation_id;longitude',
      'as' => 'nd_geo_lon',
    ]);
    $properties[] = new ChadoRealStoragePropertyType($entity_type_id, self::$id, 'nd_geo_alt', $altitude_term, [
      'action' => 'read_value',
      'drupal_store' => FALSE,
      'path' => $linker_table . '.' . $linker_fkey_column . '>' . $object_table . '.' . $object_pkey_col
      . ';' . $object_table . '.nd_geolocation_id>nd_geolocation.nd_geolocation_id;altitude',
      'as' => 'nd_geo_alt',
    ]);
    $properties[] = new ChadoVarCharStoragePropertyType($entity_type_id, self::$id, 'nd_geodetic_datum', $geodetic_datum_term, $geodetic_datum_len, [
      'action' => 'read_value',
      'drupal_store' => FALSE,
      'path' => $linker_table . '.' . $linker_fkey_column . '>' . $object_table . '.' . $object_pkey_col
      . ';' . $object_table . '.nd_geolocation_id>nd_geolocation.nd_geolocation_id;geodetic_datum',
      'as' => 'nd_geodetic_datum',
    ]);
    $properties[] = new ChadoTextStoragePropertyType($entity_type_id, self::$id, 'nd_geo_desc', $description_term, [
      'action' => 'read_value',
      'drupal_store' => FALSE,
      'path' => $linker_table . '.' . $linker_fkey_column . '>' . $object_table . '.' . $object_pkey_col
      . ';' . $object_table . '.nd_geolocation_id>nd_geolocation.nd_geolocation_id;description',
      'as' => 'nd_geo_desc',
    ]);

    return $properties;
  }

  /**
   * {@inheritdoc}
   */
  public static function generateSampleValue(FieldDefinitionInterface $field_definition) {
    $values = [];
    $values['record_id'] = 1;
    $values['entity_id'] = NULL;
    $values['linker_type_id'] = 1;
    $values['nd_exp_type_id'] = 1;
    $values['nd_geo_lat'] = 0.0;
    $values['nd_geo_lon'] = 0.0;
    $values['nd_geo_alt'] = 0.0;
    $values['nd_geodetic_datum'] = '';
    $values['nd_geo_desc'] = '';
    return $values;
  }

  /**
   * {@inheritdoc}
   */
  public function getConstraints() {
    $constraints = parent::getConstraints();

    /**
     * Ensure that the value entered is not larger then the max length.
     * @code
     * if ($max_length = $this->getSetting('max_length')) {
     *   $constraint_manager = \Drupal::typedDataManager()->getValidationConstraintManager();
     *   $constraints[] = $constraint_manager->create('ComplexData', [
     *     'value' => [
     *       'Length' => [
     *         'max' => $max_length,
     *         'maxMessage' => t('%name: may not be longer than @max characters.', [
     *           '%name' => $this
     *           ->getFieldDefinition()
     *           ->getLabel(),
     *           '@max' => $max_length,
     *         ]),
     *       ],
     *     ],
     *   ]);
     * }
     * @endcode
     */

    return $constraints;
  }

  /**
   * {@inheritDoc}
   *
   * @see \Drupal\tripal_chado\TripalField\ChadoFieldItemBase::isCompatible()
   */
  public function isCompatible(TripalEntityType $entity_type) : bool {
    $compatible = TRUE;

    // Get the base table for the content type.
    $base_table = $entity_type->getThirdPartySetting('tripal', 'chado_base_table');
    $linker_tables = $this->getLinkerTables(self::$object_table, $base_table);
    $table_columns = $this->getTableColumns($base_table, self::$valid_base_column_types);
    if (count($linker_tables) < 1) {
      $compatible = FALSE;
    }
    return $compatible;
  }

  /**
   * {@inheritDoc}
   *
   * @see \Drupal\tripal\TripalField\Interfaces\TripalFieldItemInterface::discover()
   */
  public static function discover(
    TripalEntityType $bundle,
    string $field_id,
    array $field_types,
    array $field_instances,
    array $options = [],
  ): array {

    // Specific settings for this field.
    $options += [
      'id' => self::$id,
      'table' => self::$object_table,
      'label' => 'Geographic Location',
      'termIdSpace' => 'SIO',
      'termAccession' => '000013',
      'description' => 'A geo-referenceable location',
    ];

    // Call the parent discover() with this field's specific options.
    $field_list = parent::discover($bundle, $field_id, $field_types, $field_instances, $options);

    return $field_list;
  }

}
