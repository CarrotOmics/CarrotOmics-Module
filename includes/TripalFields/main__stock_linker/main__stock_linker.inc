<?php

/**
 * @class
 * Purpose:
 *
 * The linker table version of main__stock
 *
 * Data:
 * Assumptions:
 */
class main__stock_linker extends ChadoField {

  /**
   * The default label for this field.
   */
  public static $default_label = 'Stock';

  /**
   * The default description for this field.
   */
  public static $default_description = 'Stock associated with this entity';

  /**
   * The default widget for this field.
   */
  public static $default_widget = 'main__stock_linker_widget';

  /**
   * The default formatter for this field.
   */
  public static $default_formatter = 'main__stock_linker_formatter';

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
    'term_vocabulary' => 'MAIN',
    // The name of the term.
    'term_name' => 'stock',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'stock',
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
      drupal_set_message(t('The stock linker field for ' . $field_table . ' is not present in the chado database!'));
    }
    $pkey = $schema['primary key'][0];

    $linker = 'chado.' . $field_table;
    $linker_key = $field_table . '_id';
    $fkey_lcolumn = key($schema['foreign keys'][$base_table]['columns']);
    $fkey_rcolumn = $schema['foreign keys'][$base_table]['columns'][$fkey_lcolumn];

//    if (($base_table === 'library') or ($base_table === 'project')) {
//      $linker = 'chado.xxxorganism_' . $base_table;
//      $linker_key = $base_table . '_stock_id';
//    }

    $record_id = $entity_record->$key;
    $sql = "SELECT S.stock_id, S.organism_id, S.name, S.uniquename, S.description, CV.name AS stocktype, $linker_key"
         . " FROM {stock} S"
         . " LEFT JOIN $linker L ON L.stock_id=S.stock_id"
         . " LEFT JOIN {cvterm} CV ON CV.cvterm_id=S.type_id"
         . " WHERE L.$fkey_lcolumn=:record_id";
    $args = [ ':record_id' => $record_id ];
    $results = chado_query( $sql, $args );

    $i = 0;
    while ( $result = $results->fetchObject() ) {

    
      $stock_id = $result->stock_id;
      $stock_name = $result->name;
      $stock_uniquename = $result->uniquename;
      $stock_type = $result->stocktype;
      $stock_description = $result->description;
      $stock_organism_id = $result->organism_id;
      $stock_organism_name = chado_get_organism_scientific_name(chado_get_organism(['organism_id' => $stock_organism_id], []));
      $linker_record_id = $result->$linker_key;

      $stock_entity_id = chado_get_record_entity_by_table('stock', $stock_id);

      $entity->{$field_name}['und'][$i] = [
        'value' =>
          [
            'entity_id' => $stock_entity_id,
            'type_name' => $stock_type,
            'name' => $stock_name,
            'uniquename' => $stock_uniquename,
            'description' => $stock_description,
            'organism_name' => $stock_organism_name,
            'organism_id' => $stock_organism_id,
            'linker_record_id' => $linker_record_id,
          ],
        'chado-' . $field_table . '__' . $pkey => $linker_record_id,
        'chado-' . $field_table . '__' . 'stock_id' => $stock_id,
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
