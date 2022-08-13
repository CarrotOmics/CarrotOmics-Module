<?php

class local__project_analysis extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class. They are
  // used by the static functions to provide information to Drupal about
  // the field and its default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'project_analysis';

  // The default description for this field.
  public static $description = 'Related project or analysis';

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
    'term_name' => 'project_analysis',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'project_analysis',
    // Set to FALSE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
    // The table in Chado that the instance maps to.
    'chado_table' => 'analysis',
    // The primary key column of the table in Chado.
    'chado_column' => 'analysis_id',
    // The base table.
    'base_table' => 'analysis',

  ];

  // The default widget for this field.
  public static $default_widget = 'local__project_analysis_widget';

  // The default formatter for this field.
  public static $default_formatter = 'local__project_analysis_formatter';

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

    $name_term = chado_get_semweb_term('analysis', 'name');
    $element_term = 'local:project_analysis';

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

    // find all analyses and assays linked to this project
    $analysis_ids = [];
    $assay_ids = [];
    $sql1 = "SELECT PA.analysis_id FROM {project_analysis} PA"
          . " WHERE project_id=:project_id";
    $sql2 = "SELECT AP.assay_id FROM {assay_project} AP"
          . " WHERE project_id=:project_id";
    $args = [ ':project_id' => $project_id ];
    $records1 = chado_query( $sql1, $args );
    $records2 = chado_query( $sql2, $args );
    if ( (!$records1) and (!$records2) ) {
      return;  // failure in query, should not happen?
    }
    // simplify results down to an array of just the analysis_id values
    while ( $record = $records1->fetchObject() ) {
      $analysis_ids[] = $record->analysis_id;
    }
    while ( $record = $records2->fetchObject() ) {
      $assay_ids[] = $record->assay_id;
    }
    if ( (!$analysis_ids) and (!$assay_ids) ) {
      return;  // no analyses or assays found
    }

    // we now have one or more analysis_id or assay_id values. Look up entities for them.
    // I could use chado_get_record_entity_by_table(), but I also want the information
    // from chado_bundle e.g. genome_assembly instead of just analysis.
    // First get a list of possible bundles using the analysis or assay tables.
    $analysis_bundles = [];
    $assay_bundles = [];
    $sql3 = "SELECT bundle_id, data_table, type_linker_table, type_column,"
          . " (SELECT name FROM {cvterm} WHERE cvterm_id=type_id) AS type, type_value"
          . " FROM [chado_bundle] WHERE data_table = :table";
    $records3 = chado_query( $sql3, [':table' => 'analysis'] );
    $records4 = chado_query( $sql3, [':table' => 'assay'] );
    if ($records3) {
      while ( $record = $records3->fetchObject() ) {
        $analysis_bundles[] = [ $record->bundle_id, $record->data_table, $record->type, $record->type_value ];
      }
    }
    if ($records4) {
      while ( $record = $records4->fetchObject() ) {
        $assay_bundles[] = [ $record->bundle_id, $record->data_table, $record->type, $record->type_value ];
      }
    }

    // Scan each of the bundle tables for analyses found above
    // To-do: Bunch of duplication here, make a function
    $analysis_str = implode(',', $analysis_ids);
    $assay_str = implode(',', $assay_ids);
    $delta = 0;
    if ($analysis_str) {
      foreach ($analysis_bundles as $tableref) {
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
        // can't put analysis_str in $args because it will be quoted
        $sql = 'SELECT B.entity_id, A.name FROM ['.$bundletable.'] B'
             . ' INNER JOIN {analysis} A on B.record_id = A.analysis_id'
             . ' WHERE B.record_id IN ('.$analysis_str.')';
        $args = [];
        $records = chado_query($sql, $args);
        if ($records) {
          while ( $record = $records->fetchObject() ) {
            $eid = $record->entity_id;
            $name = $record->name;
            $entity->{$field_name}['und'][$delta]['value']['rdfs:label'] = $type;
            $entity->{$field_name}['und'][$delta]['value']['schema:name'] = $name;
            if ($eid) {
              $entity->{$field_name}['und'][$delta]['value']['entity'] = 'TripalEntity:' . $eid;
            }
            $delta++;
          }
        }
      }
    }
    if ($assay_str) {
      foreach ($assay_bundles as $tableref) {
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
        // can't put assay_str in $args because it will be quoted
        $sql = 'SELECT B.entity_id, A.name FROM ['.$bundletable.'] B'
             . ' INNER JOIN {assay} A on B.record_id = A.assay_id'
             . ' WHERE B.record_id IN ('.$assay_str.')';
        $args = [];
        $records = chado_query($sql, $args);
        if ($records) {
          while ( $record = $records->fetchObject() ) {
            $eid = $record->entity_id;
            $name = $record->name;
            $entity->{$field_name}['und'][$delta]['value']['rdfs:label'] = $type;
            $entity->{$field_name}['und'][$delta]['value']['schema:name'] = $name;
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
