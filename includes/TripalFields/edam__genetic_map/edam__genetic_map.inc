<?php

/**
 * @class
 * Purpose:
 *
 * Tripal field to display associated Genetic Maps
 *
 * Data:
 * Assumptions:
 */
class edam__genetic_map extends ChadoField {

  /**
   * The default label for this field.
   */
  public static $default_label = 'Genetic Map';

  /**
   * The default description for this field.
   */
  public static $default_description = 'Genetic map associated with this entity';

  /**
   * The default widget for this field.
   */
  public static $default_widget = 'edam__genetic_map_widget';

  /**
   * The default formatter for this field.
   */
  public static $default_formatter = 'edam__genetic_map_formatter';

  // The module that manages this field.
  // If no module manages the field (i.e. it's added via libraries)
  // then set this to 'tripal_chado'
  public static $module = 'carrotomics';

  public static $default_settings = [
    'storage' => 'field_chado_storage',
    'searchable_keys' => [],
  ];

  /**
   * Required for all TripalFields.
   */
  public static $default_instance_settings = [
    // The DATABASE name, as it appears in chado.db. This also builds the link-out url.
    // In most cases this will simply be the CV name. In some cases (EDAM) this will be the SUBONTOLOGY.
    'term_vocabulary' => 'data',
    // The name of the term.
    'term_name' => 'Genetic Map',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '1278',
    'term_fixed' => FALSE,
    'auto_attach' => FALSE,
    'download_formatters' => [
      'TripalTabDownloader',
      'TripalCSVDownloader',
    ],
  ];

  public static $no_ui = FALSE;

  public static $no_data = FALSE;

  /**
   * Load the field.
   *
   * @see ChadoField::load()
   */
  public function load($entity) {

    parent::load($entity);

    $entity_record = $entity->chado_record;
    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];
    $base_table = $this->instance['settings']['base_table'];
    $key = $base_table . '_id';

    $schema = chado_get_schema($field_table);
    if (!$schema) {
      drupal_set_message(t('The featuremap linker field for ' . $field_table . ' is not present in the chado database!'));
    }
    $pkey = $schema['primary key'][0];

    $linker = 'chado.' . $field_table;
    $linker_key = $field_table . '_id';
    $fkey_lcolumn = key($schema['foreign keys'][$base_table]['columns']);
    $fkey_rcolumn = $schema['foreign keys'][$base_table]['columns'][$fkey_lcolumn];

    $record_id = $entity_record->$key;
    $sql = "SELECT FM.featuremap_id, FM.name, FM.description, FM.unittype_id, FP.value AS featuremaptype, $linker_key"
         . " FROM {featuremap} FM"
         . " LEFT JOIN $linker L ON L.featuremap_id=FM.featuremap_id"
         . " LEFT JOIN {featuremapprop} FP ON FM.featuremap_id=FP.featuremap_id"
         . " LEFT JOIN {cvterm} CV ON CV.cvterm_id=FP.type_id"
         . " WHERE FP.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name='type'"
         .   " AND cv_id=(SELECT cv_id FROM {cv} WHERE name='rdfs'))"
         . " AND L.$fkey_lcolumn=:record_id";
    $args = [ ':record_id' => $record_id ];
    $results = chado_query( $sql, $args );

    $i = 0;
    while ( $result = $results->fetchObject() ) {

    
      $featuremap_id = $result->featuremap_id;
      $featuremap_name = $result->name;
      $featuremap_type = $result->featuremaptype;
      $featuremap_description = $result->description;
      $linker_record_id = $result->$linker_key;

      $featuremap_entity_id = chado_get_record_entity_by_table('featuremap', $featuremap_id);

      $entity->{$field_name}['und'][$i] = [
        'value' =>
          [
            'entity_id' => $featuremap_entity_id,
            'type_name' => $featuremap_type,
            'name' => $featuremap_name,
            'description' => $featuremap_description,
            'linker_record_id' => $linker_record_id,
          ],
        'chado-' . $field_table . '__' . $pkey => $linker_record_id,
        'chado-' . $field_table . '__' . 'featuremap_id' => $featuremap_id,
        'chado-' . $field_table . '__' . $fkey_lcolumn => $record_id,

      ];

      ++$i;

    }
  }

  /**
   * Query.
   *
   * @see ChadoField::query()
   */
  public function query($query, $condition) {
  }

  /**
   * Query Order.
   *
   * @see ChadoField::queryOrder()
   */
  public function queryOrder($query, $order) {
  }

  /**
   * Element Info.
   *
   * @see ChadoField::elementInfo()
   */
  public function elementInfo() {
    $field_term = $this->getFieldTermID();
    return [
      $field_term => [
        'operations' => ['eq', 'ne', 'contains', 'starts'],
        'sortable' => TRUE,
        'searchable' => TRUE,
      ],
    ];
  }

}
