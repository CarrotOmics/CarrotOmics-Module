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

    // find all supported linked information types linked to this project
    $linkers = ['analysis' => 'project_analysis',
                'assay' => 'assay_project',
                'featuremap' => 'project_featuremap'];
    $ids = [];
    foreach ($linkers as $table => $linkertable) {
      $idcolumn = $table . '_id';
      $sql1 = 'SELECT '.$idcolumn.' AS idvalue FROM {'.$linkertable.'}'
            . ' WHERE project_id=:project_id';
      $args1 = [':project_id' => $project_id];
      $records1 = chado_query( $sql1, $args1 );
      // simplify results down to an array of just the type and id values
      while ( $record1 = $records1->fetchObject() ) {
        $ids[$table][] = $record1->idvalue;
      }
    }

    if (!$ids) {
      return;  // no links of any type found
    }

    // We now have one or more id values. Look up entities for them.
    // I could use chado_get_record_entity_by_table(), but I also want the information
    // from chado_bundle e.g. genome_assembly instead of just analysis.
    // First get a list of possible bundles using the relevant table, e.g. analysis.
    $bundles = [];
    foreach ($linkers as $table => $linkertable) {
      $sql2 = 'SELECT bundle_id, data_table, type_linker_table, type_column,'
            . ' (SELECT name FROM {cvterm} WHERE cvterm_id=type_id) AS type, type_value'
            . ' FROM [chado_bundle] WHERE data_table = :table';
      $args2 = [':table' => $table];
      $records2 = chado_query( $sql2, $args2 );
      if ($records2) {
        while ( $record2 = $records2->fetchObject() ) {
          // $record->data_table is the same as $table at this point
          $bundles[$table][] = [ $record2->bundle_id, $record2->type, $record2->type_value ];
        }
      }
    }

    // Scan each of the bundle tables for linked records found above
    $delta = 0;
    foreach ($linkers as $table => $linkertable) {
      if (array_key_exists($table, $ids) and array_key_exists($table, $bundles)) {
        $idlist = implode(',', $ids[$table]);
        foreach ($bundles[$table] as $tableref) {

          $bid = $tableref[0];
          // In some cases the type is just 'type' in which case then
          // use type_value e.g. genome_assembly
          $contenttype = $tableref[1];
          if ((!$contenttype) or ($contenttype == 'type')) {
            $contenttype = $tableref[2];
          }
          // Special case handling for maps
          if (($table == 'featuremap') and (!preg_match('/map/i', $contenttype))) {
            $contenttype .= ' Map';
          }
          // Format for nicer display e.g. "genome_assembly" becomes "Genome Assembly"
          $contenttype = ucwords(preg_replace('/_/', ' ', $contenttype));

          $bundletable = "chado_bio_data_" . $bid;
          // can't put our imploded $idlist string in $args because it will be quoted
          $sql3 = 'SELECT B.entity_id, A.name FROM ['.$bundletable.'] B'
                . ' INNER JOIN {'.$table.'} A on B.record_id = A.'.$table.'_id'
                . ' WHERE B.record_id IN ('.$idlist.')';
          $args3 = [];
          $records3 = chado_query($sql3, $args3);
          if ($records3) {
            while ( $record3 = $records3->fetchObject() ) {
              $eid = $record3->entity_id;
              $name = $record3->name;
              $entity->{$field_name}['und'][$delta]['value']['rdfs:label'] = $contenttype;
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
}
