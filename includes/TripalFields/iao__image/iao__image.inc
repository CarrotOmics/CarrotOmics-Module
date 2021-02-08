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
    'term_vocabulary' => 'iao',
    // The name of the term.
    'term_name' => 'image',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '0000101',
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
//file_put_contents( "/tmp/log.carrotomics", "CP3a elementInfo\n", FILE_APPEND ); //@@@

    $image_term = 'IAO:image';
    // eimage_prop supplies all other linked information

    return [
      $field_term => [
        'operations' => [],
        'sortable' => FALSE,
        'searchable' => FALSE,
        'type' => 'xs:complexType',
        'readonly' => FALSE,
        'elements' => [
          'iao:image' => [
            'name' => 'image',
          ],
          $image_term => [
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

//file_put_contents( "/tmp/log.carrotomics", "CP4b load()\n", FILE_APPEND ); //@@@
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

//ob_start(); var_dump($chado_record); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP4b load() chado_record = ".$astring."\n", FILE_APPEND ); //@@@
    // Using the nd_experiment_stock table, find all nd_geolocation records
    // linked to this record
//file_put_contents( "/tmp/log.carrotomics", "CP4c load() stock_id=".$chado_record->stock_id."\n", FILE_APPEND ); //@@@
    $records = chado_generate_var('stock_image',
                                  ['stock_id' => $chado_record->stock_id],
                                  ['return_array' => TRUE]);
//ob_start(); var_dump($records); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP4d load() records=".$astring."\n", FILE_APPEND ); //@@@
    // iterate to retrieve all eimage_id values for this stock_id
    $one_eimage_record = [];
    foreach ($records as $record) {
      if ($record->eimage_id) {
        $eimage_id = $record->eimage_id->eimage_id;
        $image_uri = $record->eimage_id->image_uri;
        $one_eimage_record = [ 'id' => $eimage_id, 'uri' => $image_uri, 'annotations' => [] ];
//file_put_contents( "/tmp/log.carrotomics", "CP4e load() eimage_id=$eimage_id\n", FILE_APPEND ); //@@@

        // each image may have zero or more annotations in the eimageprop table
        $annotations = chado_generate_var('eimageprop',
                                          ['eimage_id' => $eimage_id],
                                          ['return_array' => TRUE]);
        foreach ($annotations as $annotation) {
//ob_start(); var_dump($annotation); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP4e load() annotation = ".$astring."\n", FILE_APPEND ); //@@@
          $label = $annotation->type_id->name;  // the CV term for this annotation
//          $cv = $annotation->type_id->cv_id->name;
//          $label = $cv . ':' . $label;  // the CV and CV term for this annotation
          $value = $annotation->value;  // the value of the annotation
          $one_eimage_record['annotations'][$label] = $value;
//file_put_contents( "/tmp/log.carrotomics", "CP4f load() lbl=$label val=$value\n", FILE_APPEND ); //@@@
          
        }

        // add everything about this one image to the entity array as another 'value' element
        $entity->{$field_name}['und'][] = ['value' => $one_eimage_record];
//ob_start(); var_dump($entity->{$field_name}['und']); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP4g load() und = ".$astring."\n", FILE_APPEND ); //@@@
      }
    }
  }
}



