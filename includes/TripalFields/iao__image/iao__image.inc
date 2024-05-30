<?php

class iao__image extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class. They are
  // used by the static functions to provide information to Drupal about
  // the field and its default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'Image File';

  // The default description for this field.
  public static $description = 'Image of a plant or its parts.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = [
    // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'IAO',  // DB name
    // The name of the term.
    'term_name' => 'image',  // cvterm name
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '0000101',  // DB accession from dbxref from the cvterm
    // Set to FALSE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => 'eimage',
    // The primary key column of the table in Chado.
    'chado_column' => 'eimage_id',
    // The base table.
    'base_table' => 'eimage',

  ];

  // The default widget for this field.
  public static $default_widget = 'iao__image_widget';

  // The default formatter for this field.
  public static $default_formatter = 'iao__image_formatter';

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

    $uri_term = chado_get_semweb_term('eimage', 'image_uri');
    // eimage_prop supplies all other linked information

    return [
      $field_term => [
        'operations' => [],
        'sortable' => FALSE,
        'searchable' => FALSE,
        'type' => 'xs:complexType',
        'readonly' => FALSE,
        'elements' => [
          $uri_term => [
            'name' => 'Image',
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

    $settings = $this->field['settings'];

    $record = $entity->chado_record;
    $record_id = $entity->chado_record_id;

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];
    $base_table = $this->instance['settings']['base_table'];  // eimage
    $host_table = $record->tablename;
    $host_pkey = $host_table . '_id';  // should look this up

    // Get the linker table
    $linker_table = $host_table . '_image';

    // If we don't have a chado record return before creating a stub for this field!
    if (!$record) {
      return;
    }

    // Set some defaults for the empty record.
    $chado_record = $entity->chado_record;
    $entity->{$field_name}['und'][0]['value'] = [];

    // look up id values of all of the stock bundles
    $acbundles = [];
    $sql = "SELECT CB.bundle_id, TB.label FROM [chado_bundle] CB"
         . " LEFT JOIN [tripal_bundle] TB ON TB.name = CONCAT('bio_data_', CB.bundle_id)"
         . " WHERE CB.data_table = 'stock'";
    $records = chado_query( $sql, [] );
    if ( $records ) {
      while ( $record = $records->fetchObject() ) {
        $acbundles[$record->bundle_id] = $record->label;
      }
    }

    // Using the linker table, find all eimage records linked
    // to this host record
    $object_pkey = $chado_record->$host_pkey;
    $records = chado_generate_var($linker_table,
                                  [$host_pkey => $object_pkey],
                                  ['return_array' => TRUE]);

    // iterate to retrieve all eimage_id values for this host content id
    $one_eimage_record = [];
    $delta = 0;
    foreach ($records as $record) {
      if ($record->eimage_id) {
        $eimage_id = $record->eimage_id->eimage_id;
        $image_uri = $record->eimage_id->image_uri;
        $one_eimage_record = [ 'id' => $eimage_id, 'uri' => $image_uri, 'annotations' => [] ];

        // each image may have zero or more annotations in the eimageprop table
        $annotations = chado_generate_var('eimageprop',
                                          ['eimage_id' => $eimage_id],
                                          ['return_array' => TRUE]);
        foreach ($annotations as $annotation) {
          $label = $annotation->type_id->name;  // the CV term for this annotation
          $value = $annotation->value;  // the value of the annotation
          if ($value) {
            $one_eimage_record['annotations'][$label] = $value;
          }
        }

        // special case: link image back to stock if possible.
        // This hardcodes the link - not desirable, but wait until Tripal 4 for a better method
        // @todo include biomaterial also?
        $one_eimage_record = $this->getStocks($eimage_id, $acbundles, $one_eimage_record);

        // add everything about this one image to the entity array as another 'value' element
        $entity->{$field_name}['und'][$delta]['value'] = $one_eimage_record;
        $delta++;
      }
    }
  }

  protected function getStocks($eimage_id, $acbundles, $hostrecord) {
    // Consult the linking table for any linked stocks
    $sql = "SELECT S.stock_id, S.uniquename FROM {stock_image} SI"
         . " LEFT JOIN {stock} S ON SI.stock_id=S.stock_id"
         . " WHERE SI.eimage_id = :eimage_id";
    $args = [ ":eimage_id" => $eimage_id ];
    $records = chado_query($sql, $args);

    // Process any stocks that were found and add entities if possible
    if ( $records ) {
      while ( $record = $records->fetchObject() ) {
        $stock_id = $record->stock_id;
        $stock_name = $record->uniquename;

        // Get entity id values
        $found_entity = FALSE;
        foreach ($acbundles as $acbundle => $entity_name) {
          $bsql = 'SELECT entity_id FROM [chado_bio_data_' . $acbundle . '] WHERE record_id = :stock_id';
          $bargs = [':stock_id' => $stock_id];
          $brecords = chado_query($bsql, $bargs);

          // not all stock types may have entities, only add a link to those that do
          if ( $brecords ) {
            while ($brecord = $brecords->fetchObject()) {
              $link = '<a href="/bio_data/' . $brecord->entity_id . '">' . $stock_name . '</a>';
              $hostrecord['annotations'][$entity_name] = $link;
              $found_entity = TRUE;
            }
          }
        }
        // If there is a stock but no entity, then just display the name
        if (!$found_entity) {
          $hostrecord['annotations']['stock'] = $stock_name;
        }
      }
    }
    return $hostrecord;
  }
}



