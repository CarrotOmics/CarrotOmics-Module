<?php

class edam__phylogenetic_tree extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class. They are
  // used by the static functions to provide information to Drupal about
  // the field and its default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'Phylogenetic Tree';

  // The default description for this field.
  public static $description = 'Phylogenetic tree for taxonomy, germplasm, gene families, etc.';

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
    'term_name' => 'Phylogenetic tree',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'data:0872',
    // Set to FALSE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => 'phylotree',
    // The primary key column of the table in Chado.
    'chado_column' => 'phylotree_id',
    // The base table.
    'base_table' => 'phylotree',

  ];

  // The default widget for this field.
  public static $default_widget = 'edam__phylogenetic_tree_widget';

  // The default formatter for this field.
  public static $default_formatter = 'edam__phylogenetic_tree_formatter';

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

    $element_term = 'data:0872';
    // phylotree_prop supplies all other linked information

    return [
      $field_term => [
        'operations' => [],
        'sortable' => FALSE,
        'searchable' => FALSE,
        'type' => 'xs:complexType',
        'readonly' => FALSE,
        'elements' => [
          $element_term => [
            'name' => 'Phylogenetic tree',
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
    $base_table = $this->instance['settings']['base_table'];

    // If we don't have a chado record return before creating a stub for this field!
    if (!$record) {
      return;
    }

    // Set some defaults for the empty record.
    $chado_record = $entity->chado_record;
    $entity->{$field_name}['und'][0]['value'] = [];

    // look up id values of phylotree bundle(s) - should only be one, though
    $ptbundles = [];
    $sql = "SELECT bundle_id FROM [chado_bundle] WHERE data_table = 'phylotree'";
    $records = chado_query( $sql, [] );
    if ( $records ) {
      while ( $record = $records->fetchObject() ) {
        $ptbundles[] = $record->bundle_id;
      }
    }

    // The phylotree table contains the corresponding analysis_id
    $sql = "SELECT phylotree_id, name FROM {phylotree} WHERE analysis_id = :analysis_id";
    $args = [ ":analysis_id" => $chado_record->analysis_id ];
    $records = chado_query( $sql, $args );

    // for each phylotree linked to this analysis, get the tree name and id, and the entity_id
    $delta = 0;
    if ( $records ) {
      while ( $record = $records->fetchObject() ) {
        //$phylotree_id = $record->phylotree_id;
        $phylotree_name = $record->name;

        // get entity id values.
        foreach ($ptbundles as $ptbundle) {
          $bsql = "SELECT entity_id FROM [chado_bio_data_" . $ptbundle . "] WHERE record_id = :phylotree_id";
          $bargs = [ ":phylotree_id" => $record->phylotree_id ];
          $brecords = chado_query( $bsql, $bargs );

          if ( $brecords ) {
            while ( $brecord = $brecords->fetchObject() ) {
              $entity->{$field_name}['und'][$delta]['value']['rdfs:label'] = $phylotree_name;
              $entity->{$field_name}['und'][$delta]['value']['entity'] = 'Phylogenetictree:' . $brecord->entity_id;
              $delta++;
            }
          }
        }
      }
    }
  }
}
