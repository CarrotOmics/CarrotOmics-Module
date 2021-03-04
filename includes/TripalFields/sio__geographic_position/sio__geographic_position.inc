<?php

class sio__geographic_position extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class. They are
  // used by the static functions to provide information to Drupal about
  // the field and its default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'Geographic position';

  // The default description for this field.
  public static $description = 'Geographic position coordinates: latitude, longitude, and altitude.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = [
    // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'SIO',
    // The name of the term.
    'term_name' => 'geographic position',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '000013',
    // Set to FALSE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => 'nd_geolocation',
    // The primary key column of the table in Chado.
    'chado_column' => 'nd_geolocation_id',
    // The base table.
    'base_table' => 'nd_geolocation',

  ];

  // The default widget for this field.
  public static $default_widget = 'sio__geographic_position_widget';

  // The default formatter for this field.
  public static $default_formatter = 'sio__geographic_position_formatter';

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;



  /**
   * @see TripalField::elementInfo()
   */
  public function elementInfo() {
    $field_term = $this->getFieldTermID();

    $description_term = chado_get_semweb_term('nd_geolocation', 'description'); // i.e. schema:description
    $latitude_term = chado_get_semweb_term('nd_geolocation', 'latitude') ?? 'SIO:000319';
    $longitude_term = chado_get_semweb_term('nd_geolocation', 'longitude') ?? 'SIO:000318';
    $altitude_term = chado_get_semweb_term('nd_geolocation', 'altitude') ?? 'SIO:000438';
    $latitude_dev_term = chado_get_semweb_term('nd_geolocation', 'latitude_dev') ?? 'local:latitude_dev';
    $longitude_dev_term = chado_get_semweb_term('nd_geolocation', 'longitude_dev') ?? 'local:longitude_dev';
    $altitude_dev_term = chado_get_semweb_term('nd_geolocation', 'altitude_dev') ?? 'local:altitude_dev';

    return [
      $field_term => [
        'operations' => ['eq', 'gt', 'lt', 'gte', 'lte'],
        'sortable' => FALSE,
        'searchable' => FALSE,
        'type' => 'xs:complexType',
        'readonly' => FALSE,
        'elements' => [
          'rdfs:label' => [
            'name' => 'label',
          ],
          $description_term => [
            'name' => 'description',
            'operations' => ['eq', 'ne', 'contains', 'starts'],
          ],
          $latitude_term => [
            'name' => 'latitude',
          ],
          $longitude_term => [
            'name' => 'longitude',
          ],
          $altitude_term => [
            'name' => 'altitude',
          ],
          $latitude_dev_term => [
            'name' => 'latitude deviation',
          ],
          $longitude_dev_term => [
            'name' => 'longitude deviation',
          ],
          $altitude_dev_term => [
            'name' => 'altitude deviation',
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

    $record = $entity->chado_record;
    $record_id = $entity->chado_record_id;

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];
    $base_table = $this->instance['settings']['base_table'];

    // Set some defaults for the empty record.
    $chado_record = $entity->chado_record;
    $entity->{$field_name}['und'][0] = [
      'value' => '',
    ];

    // Find all nd_geolocation records linked to this record
    // Lookups Implemented: nd_experiment_analysis  nd_experiment_contact
    //                      nd_experiment_project nd_experiment_stock
    // Not implemented: nd_experiment_dbxref  nd_experiment_genotype  nd_experiment_phenotype 
    //                  nd_experiment_protocol  nd_experiment_pub  
    // perform a direct sql query instead of chado_generate_var here for better
    // efficiency, e.g. if a large number of stocks exist in a project
    if ( property_exists($chado_record, 'analysis_id') ) {
      $sql = "SELECT DISTINCT nd_geolocation_id FROM {nd_experiment_analysis} A INNER JOIN"
        . " {nd_experiment} L ON L.nd_experiment_id=A.nd_experiment_id WHERE A.analysis_id=:id";
      $args = [':id' => $chado_record->analysis_id];
    } elseif ( property_exists($chado_record, 'contact_id') ) {
      $sql = "SELECT DISTINCT nd_geolocation_id FROM {nd_experiment_contact} C INNER JOIN"
        . " {nd_experiment} L ON L.nd_experiment_id=C.nd_experiment_id WHERE C.analysis_id=:id";
      $args = [':id' => $chado_record->contact_id];
    } elseif ( property_exists($chado_record, 'project_id') ) {
      $sql = "SELECT DISTINCT nd_geolocation_id FROM {nd_experiment_project} P INNER JOIN"
        . " {nd_experiment} L ON L.nd_experiment_id=P.nd_experiment_id WHERE P.project_id=:id";
      $args = [':id' => $chado_record->project_id];
    } elseif ( property_exists($chado_record, 'stock_id') ) {
      $sql = "SELECT DISTINCT nd_geolocation_id FROM {nd_experiment_stock} S INNER JOIN"
        . " {nd_experiment} L ON L.nd_experiment_id=S.nd_experiment_id WHERE S.stock_id=:id";
      $args = [':id' => $chado_record->stock_id];
    }
    if ($sql) {
      $idrecords = chado_query($sql, $args);
      if ( $idrecords ) {
        while($idrecord = $idrecords->fetchObject()) {
          $records = chado_generate_var('nd_geolocation',
                                        ['nd_geolocation_id' => $idrecord->nd_geolocation_id],
                                        ['return_array' => TRUE]);
          foreach ($records as $record) {
            // Get the terms for each of the keys for the 'values' properties.
            $label_term = 'rdfs:label';
            $description_term = chado_get_semweb_term('nd_geolocation', 'description'); // i.e. schema:description
            $latitude_term = chado_get_semweb_term('nd_geolocation', 'latitude') ?? 'SIO:latitude';
            $longitude_term = chado_get_semweb_term('nd_geolocation', 'longitude') ?? 'SIO:longitude';
            $altitude_term = chado_get_semweb_term('nd_geolocation', 'altitude') ?? 'SIO:altitude';
            $latitude_dev_term = chado_get_semweb_term('nd_geolocation', 'latitude_dev') ?? 'local:latitude_dev';
            $longitude_dev_term = chado_get_semweb_term('nd_geolocation', 'longitude_dev') ?? 'local:longitude_dev';
            $altitude_dev_term = chado_get_semweb_term('nd_geolocation', 'altitude_dev') ?? 'local:altitude_dev';

            $entity->{$field_name}['und'][0]['value'] = [
              $label_term => 'test-label',
              $description_term => $record->description,
              $latitude_term => $record->latitude,
              $longitude_term => $record->longitude,
              $altitude_term => $record->altitude,
              $latitude_dev_term => $record->latitude_dev,
              $longitude_dev_term => $record->longitude_dev,
              $altitude_dev_term => $record->altitude_dev,
            ];
          } // foreach $record
        } // while fetchObject
      } // if $id_records
    } // if $sql
  }
}
