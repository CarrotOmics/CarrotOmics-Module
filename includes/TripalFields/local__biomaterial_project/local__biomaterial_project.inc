<?php

class local__biomaterial_project extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class. They are
  // used by the static functions to provide information to Drupal about
  // the field and its default widget and formatter.
  // --------------------------------------------------------------------------

  // The default label for this field.
  public static $default_label = 'biomaterial_project';

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
    'term_name' => 'biomaterial_project',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'biomaterial_project',
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
  public static $default_widget = 'local__biomaterial_project_widget';

  // The default formatter for this field.
  public static $default_formatter = 'local__biomaterial_project_formatter';

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
    $element_term = 'local:biomaterial_project';

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
    $entity->{$field_name}['und'] = [];
    $biomaterial_id = $chado_record->biomaterial_id;

    // find all projects linked to this biomaterial
    $sql = "SELECT BP.project_id FROM {biomaterial_project} BP"
         . " WHERE biomaterial_id=:biomaterial_id";
    $args = [ ':biomaterial_id' => $biomaterial_id ];
    $records = chado_query( $sql, $args );
    if (!$records) {
      return;  // failure in query, should not happen?
    }
    // simplify results down to an array of just the biomaterial_id values
    $project_ids = [];
    while ( $record = $records->fetchObject() ) {
      $project_ids[] = $record->project_id;
    }
    if (!$project_ids) {
      return;  // no projects found
    }

    // we now have one or more project_id values. Extend another level to find
    // any linked analyses. There is no chado.analysis_biomaterial table so we
    // need to link through project to find them
    $project_str = implode(',', $project_ids);
    // can't put project_str in $args because it will be quoted
    $sql = 'SELECT PA.analysis_id FROM {project_analysis} PA'
         . ' WHERE project_id IN ('.$project_str.')';
    $args = [];
    $records = chado_query( $sql, $args );
    if (!$records) {
      return;  // failure in query, should not happen?
    }
    // simplify results down to an array of just the biomaterial_id values
    $analysis_ids = [];
    while ( $record = $records->fetchObject() ) {
      $analysis_ids[] = $record->analysis_id;
    }

    // Look up entities for 
    // First get a list of possible bundles using the project table.
    $pa_bundles = [];
    $sql = "SELECT bundle_id, data_table, type_linker_table, type_column,"
         . " (SELECT name FROM {cvterm} WHERE cvterm_id=type_id) AS type, type_value"
         . " FROM [chado_bundle] WHERE data_table IN ('project', 'analysis')";
    $args = [];
    $records = chado_query( $sql, $args );
    if ($records) {
      while ( $record = $records->fetchObject() ) {
        $pa_bundles[] = [ $record->bundle_id, $record->data_table, $record->type, $record->type_value ];
      }
    }

    // Scan each of the bundle tables for projects or analyses found above
    if ($analysis_ids) {
      $analysis_str = implode(',', $analysis_ids);
    }
    $delta = 0;
    foreach ($pa_bundles as $tableref) {
      $bid = $tableref[0];
      $table = $tableref[1];  // 'project' or 'analysis'
      // in some cases $type just contains 'type' in which case then use type_value e.g. genome_assembly
      $type = $tableref[2];
      if ((!$type) or ($type == 'type')) {
        $type = $tableref[3];
      }
      // Format for nicer display e.g. "genome_assembly" becomes "Genome Assembly"
      $type = ucwords(preg_replace('/_/', ' ', $type));

      $bundletable = "chado_bio_data_" . $bid;
      // customize sql depending on whether this is project or analysis
      // can't put project_str or analysis_str in $args because it will be quoted
      if ($table == 'project') {
        $sql = 'SELECT B.entity_id, P.name FROM ['.$bundletable.'] B'
             . ' INNER JOIN {project} P on B.record_id = P.project_id'
             . ' WHERE B.record_id IN ('.$project_str.')';
        $args = [];
      }        
      else if ($analysis_ids) {
        $sql = 'SELECT B.entity_id, A.name FROM ['.$bundletable.'] B'
             . ' INNER JOIN {analysis} A on B.record_id = A.analysis_id'
             . ' WHERE B.record_id IN ('.$analysis_str.')';
        $args = [];
      }
      else {
        $sql = '';
      }
      if ($sql) { 
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
