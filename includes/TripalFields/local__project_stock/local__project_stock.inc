<?php

class local__project_stock extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class. They are
  // used by the static functions to provide information to Drupal about
  // the field and its default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'project_stock';

  // The default description for this field.
  public static $description = 'Related stocks';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = [
    // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'local',
    // The name of the term.
    'term_name' => 'project_stock',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'project_stock',
    // Set to FALSE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => 'project',
    // The primary key column of the table in Chado.
    'chado_column' => 'project_id',
    // The base table.
    'base_table' => 'project',

  ];

  // The default widget for this field.
  public static $default_widget = 'local__project_stock_widget';

  // The default formatter for this field.
  public static $default_formatter = 'local__project_stock_formatter';

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

    $name_term = chado_get_semweb_term('project', 'name');
    $element_term = 'local:project_stock';

    return [
      $field_term => [
        'operations' => [],
        'sortable' => TRUE,
        'searchable' => FALSE,
        'type' => 'xs:complexType',
        'readonly' => FALSE,
        'elements' => [
          'rdfs:label' => [
             'name' => 'label',
          ],
          $name_term => [
            'name' => 'name',
            'operations' => [],
          ],
          $element_term => [
            'name' => 'entity',
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
    $project_id = $chado_record->project_id;

    // find all stocks linked to this project
    $stock_ids = [];
    $sql1 = "SELECT PS.stock_id FROM {project_stock} PS"
          . " WHERE project_id=:project_id";
    $args1 = [ ':project_id' => $project_id ];
    $records1 = chado_query( $sql1, $args1 );
    if (!$records1) {
      return;  // failure in query, should not happen?
    }

    // simplify results down to an array of just the stock_id values
    while ( $record = $records1->fetchObject() ) {
      $stock_ids[] = $record->stock_id;
    }
    if (!$stock_ids) {
      return;  // no stocks found
    }

    // we now have one or more stock_id values. Look up entities for them.
    // I could use chado_get_record_entity_by_table(), but I also want the information
    // from chado_bundle e.g. genome_project instead of just project.
    // First get a list of possible bundles using the project or assay tables.
    $stock_bundles = [];
    $sql2 = "SELECT bundle_id, data_table, type_linker_table, type_column,"
          . " (SELECT name FROM {cvterm} WHERE cvterm_id=type_id) AS type, type_value"
          . " FROM [chado_bundle] WHERE data_table = 'stock'";
    $args2 = [];
    $records2 = chado_query( $sql2, $args2 );
    if ($records2) {
      while ( $record = $records2->fetchObject() ) {
        $stock_bundles[] = [ $record->bundle_id, $record->data_table, $record->type, $record->type_value ];
      }
    }

    // Scan each of the bundle tables for stocks found above
    $stock_str = implode(',', $stock_ids);
    $delta = 0;
    if ($stock_str) {
      foreach ($stock_bundles as $tableref) {
        $bid = $tableref[0];
        $table = $tableref[1];
        // in some cases $type just contains 'type' in which case then use type_value e.g. genome_assembly
        $type = $tableref[2];
        if ((!$type) or ($type == 'type')) {
          $type = $tableref[3];
        }
        // Format for nicer display e.g. "genome_assembly" becomes "Genome Assembly"
        $type = ucwords(preg_replace('/_/', ' ', $type));

        $bundletable = "chado_bio_data_" . $bid;
        // can't put stock_str in $args because it will be quoted
        $sql3 = 'SELECT B.entity_id, A.name, A.uniquename FROM ['.$bundletable.'] B'
             . ' INNER JOIN {stock} A on B.record_id = A.stock_id'
             . ' WHERE B.record_id IN ('.$stock_str.')';
        $args3 = [];
        $records3 = chado_query($sql3, $args3);
        if ($records3) {
          while ( $record = $records3->fetchObject() ) {
            $eid = $record->entity_id;
            $name = $record->name;
            $uniquename = $record->uniquename;
            $entity->{$field_name}['und'][$delta]['value']['rdfs:label'] = $type;
            $entity->{$field_name}['und'][$delta]['value']['schema:name'] = $name;
            $entity->{$field_name}['und'][$delta]['value']['data:0842'] = $uniquename;
            if ($eid) {
              $entity->{$field_name}['und'][$delta]['value']['entity'] = 'TripalEntity:' . $eid;
            }
            $delta++;
          }
        }
      }
    }
  }
}
