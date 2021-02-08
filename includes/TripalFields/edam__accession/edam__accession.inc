<?php

class edam__accession extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class. They are
  // used by the static functions to provide information to Drupal about
  // the field and its default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'Accession';

  // The default description for this field.
  public static $description = 'Accession for germplasm, cultivar, breeding material, etc.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = [
    // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'edam',
    // The name of the term.
    'term_name' => 'Accession',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'data:2091',
    // Set to FALSE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => 'stock',
    // The primary key column of the table in Chado.
    'chado_column' => 'stock_id',
    // The base table.
    'base_table' => 'stock',

  ];

  // The default widget for this field.
  public static $default_widget = 'edam__accession_widget';

  // The default formatter for this field.
  public static $default_formatter = 'edam__accession_formatter';

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;



  /**
   * @see TripalField::elementInfo()
   */
  public function elementInfo() {
file_put_contents( "/tmp/what", "CP 2A elementInfo()\n", FILE_APPEND ); //@@@
    $field_term = $this->getFieldTermID();

    $image_term = 'edam:Accession';
    // stock_prop supplies all other linked information

    return [
      $field_term => [
        'operations' => [],
        'sortable' => TRUE,
        'searchable' => FALSE,
        'type' => 'xs:complexType',
        'readonly' => FALSE,
        'elements' => [
//          'edam:accession' => [
//            'name' => 'Germplasm Accession',
//          ],
          $image_term => [
            'name' => 'Accession',
            'operations' => [],
          ],
        ],
      ],
    ];
  }



  /**
   *
   * @see TripalField::load()
   */
  public function load($entity) {
file_put_contents( "/tmp/what", "CP 3A load()\n", FILE_APPEND ); //@@@
    $settings = $this->field['settings'];

    $record = $entity->chado_record;
    $record_id = $entity->chado_record_id;

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];
    $base_table = $this->instance['settings']['base_table'];

    // If we don't have a chado record return before creating a stub for this field!
    if (!$record) {
      return;
    }

    // Set some defaults for the empty record.
    $chado_record = $entity->chado_record;
    $entity->{$field_name}['und'] = [];

    // look up id values of stock bundles
    $acbundles = [];
    $sql = "SELECT bundle_id FROM [chado_bundle] WHERE data_table = 'stock'";
    $records = chado_query( $sql, [] );
    if ( $records ) {
      while ( $record = $records->fetchObject() ) {
        $acbundles[] = $record->bundle_id;
      }
    }

    // The stock table contains the corresponding organism_id
    $sql = "SELECT stock_id, uniquename FROM {stock} WHERE organism_id = :organism_id";
    $args = [ ":organism_id" => $chado_record->organism_id ];
    $records = chado_query( $sql, $args );

    // for each stock linked to this organism, get the uniquename, and the entity_id
    $delta = 0;
    if ( $records ) {
      while ( $record = $records->fetchObject() ) {
        $stock_name = $record->uniquename;
file_put_contents( "/tmp/what", "CP 3B load() stock_name=$stock_name\n", FILE_APPEND ); //@@@

        // get entity id values.
        foreach ($acbundles as $acbundle) {
          $bsql = "SELECT entity_id FROM [chado_bio_data_" . $acbundle . "] WHERE record_id = :stock_id";
          $bargs = [ ":stock_id" => $record->stock_id ];
          $brecords = chado_query( $bsql, $bargs );

          if ( $brecords ) {
            while ( $brecord = $brecords->fetchObject() ) {
              // not all stocks have entities, only return those that do
              if ($brecord->entity_id) {
file_put_contents( "/tmp/what", "CP 3C load() acbundle=$acbundle entity=$brecord->entity_id\n", FILE_APPEND ); //@@@
                $entity->{$field_name}['und'][$delta]['value']['rdfs:label'] = $stock_name;
                $entity->{$field_name}['und'][$delta]['value']['entity'] = 'bio_data:' . $brecord->entity_id;
                $delta++;
              }
            }
          }
        }
      }
    }
  }
}
