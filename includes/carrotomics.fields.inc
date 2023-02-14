<?php
/**
 * @file
 * Contains all field specific code outside the classes.
 */

/**
 * Implements hook_bundle_fields_info().
 *
 * Registers a field type with Drupal/Tripal. 
 * @param $entity_type -'TripalEntity' for all Tripal Content.
 * @param $bundle - Describes Tripal Entity type (Organism, Gene, etc,) hook is called for
 * 
 * @return
 *   An array of field definitions. Call tripal_refresh_bundle_fields() to trigger creation
 *   of fields for the specific bundle.
 */
function carrotomics_bundle_fields_info($entity_type, $bundle) {
  $info = [];

  $term = tripal_load_term_entity(['term_id' => $bundle->term_id]);
  $term_accession = $term->vocab->vocabulary . '__' . $term->accession;
  $table_name = $bundle->data_table;

  // if germplasm or stock entity, then we'll attach our
  // fields to display the information
  // CO_010 = Crop Germplasm Ontology
  if ( ($term_accession == 'CO_010__0000044')      // accession
    or ($term_accession == 'CO_010__0000029')      // cultivar
    or ($term_accession == 'CO_010__0000255')      // generated germplasm
    or ($term_accession == 'CO_010__0000162')      // 414 inbred line
    or ($term_accession == 'MAIN__breeding_research_material')
    or ($term_accession == 'MAIN__population') ) {

    $field_name = 'sio__geographic_position';
    $field_type = 'sio__geographic_position';
    $info[$field_name] = [
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => [
        'type' => 'field_chado_storage',
      ],
      'settings' => [],
    ];

    $field_name = 'iao__image';
    $field_type = 'iao__image';
    $info[$field_name] = [
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => [
        'type' => 'field_chado_storage',
      ],
      'settings' => [],
    ];
  }

  // for analysis entity
  if ( ($term_accession == 'operation__2945')
    or ($term_accession == 'operation__0362')
    or ($term_accession == 'operation__0525')
    or ($term_accession == 'format__1333')
    or ($term_accession == 'local__kegg_results') ) {
    $field_name = 'edam__phylogenetic_tree';
    $field_type = 'edam__phylogenetic_tree';
    $info[$field_name] = [
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => [
        'type' => 'field_chado_storage',
      ],
      'settings' => [],
    ];
  }

  // for organism entity
  if ($term_accession == 'OBI__0100026') {
    $field_name = 'edam__accession';
    $field_type = 'edam__accession';
    $info[$field_name] = [
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => [
        'type' => 'field_chado_storage',
      ],
      'settings' => [],
    ];
  }

  // same as data__sequence_coordinates
  if ( $table_name == 'feature' ) {
//@@@if I wanted to limit page types I could do something like this:
//@@@if ($term_accession == 'SO__0000234') // mRNA
//@@@    or ($term_accession == 'SO__0000704') // gene
//@@@    or ($term_accession == 'SO__0001645') ) { // marker
    $field_name = 'data__jbrowse_coordinates';
    $field_type = 'data__jbrowse_coordinates';
    $info[$field_name] = [
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => [
        'type' => 'field_chado_storage',
      ],
      'settings' => [],
    ];
  }

  return $info;
}



/**
 * Implements hook_bundle_instances_info().
 *
 * Creates a field instance of this field type on a specific Tripal Content type/bundle.
 * @param $entity_type - 'TripalEntity' for all Tripal Content.
 * @param $bundle - type of Tripal Entity (organism, gene, etc)
 * @return
 *   An array of field instance definitions. Define the defaults for any settings you use
 *   in your field.
 */
function carrotomics_bundle_instances_info($entity_type, $bundle) {
  $instances = [];
  if (!isset($bundle->data_table)) {
    return $instances;
  }

  if ($bundle->data_table == 'stock') {
    $field_name = 'sio__geographic_position';
    $field_type = 'sio__geographic_position';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Geographic Location',
      'description' => 'Geographic location of the stock or germplasm',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => TRUE,
        'chado_table' => 'nd_geolocation', // $bundle->data_table,
        'chado_column' => 'nd_geolocation_id',
        'base_table' => 'organism', // $bundle->data_table,
        'term_name' => 'geographic position',
        'term_accession' => '000013',
        'term_vocabulary' => 'SIO', // database name, not CV name
      ],
      'widget' => [
        'type' => 'sio__geographic_position_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'inline',
          'type' => 'sio__geographic_position_formatter',
          'settings' => [],
        ],
      ],
    ];

    $field_name = 'iao__image';
    $field_type = 'iao__image';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Image File',
      'description' => 'Image of a plant or its parts',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => TRUE,
        'chado_table' => 'eimage', // $bundle->data_table,
        'chado_column' => 'eimage_id',
        'base_table' => 'eimage', // $bundle->data_table,
        'term_name' => 'image',
        'term_accession' => '0000101',
        'term_vocabulary' => 'IAO',  // database name, not CV name
      ],
      'widget' => [
        'type' => 'iao__image_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'inline',
          'type' => 'iao__image_formatter',
          'settings' => [
            'title' => 'Images',
            'empty' => 'There are no images for this accession',
          ],
        ],
      ],
    ];
  }

  if ($bundle->data_table == 'analysis') {
    $field_name = 'edam__phylogenetic_tree';
    $field_type = 'edam__phylogenetic_tree';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Phylogenetic tree',
      'description' => 'Phylogenetic tree for taxonomy, germplasm, gene families, etc.',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => TRUE,
        'chado_table' => 'phylotree', // $bundle->data_table,
        'chado_column' => 'phylotree_id',
        'base_table' => 'phylotree', // $bundle->data_table,
        'term_name' => 'Phylogenetic tree',
        'term_accession' => '0872',
        'term_vocabulary' => 'data',  // database name, not CV name
      ],
      'widget' => [
        'type' => 'edam__phylogenetic_tree_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'inline',
          'type' => 'edam__phylogenetic_tree_formatter',
          'settings' => [
            'title' => 'Phylogenetic tree',
            'empty' => 'There are no phylogenetic trees for this analysis',
          ],
        ],
      ],
    ];
  }

  if ($bundle->data_table == 'feature') {
    $field_name = 'data__jbrowse_coordinates';
    $field_type = 'data__jbrowse_coordinates';
    $schema = chado_get_schema('featureloc');
    $pkey = $schema['primary key'][0];
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'View location in JBrowse',
      'description' => 'View this genome location in JBrowse',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => TRUE,
        'chado_table' => 'featureloc',
        'chado_column' => '$pkey',
        'base_table' => 'feature',
        'term_name' => 'JBrowse coordinates',
        'term_accession' => '3002',
        'term_vocabulary' => 'data',  // database name, not CV name
      ],
      'widget' => [
        'type' => 'data__jbrowse_coordinates_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'inline',
          'type' => 'data__jbrowse_coordinates_formatter',
          'settings' => [
            'title' => 'JBrowse coordinates',
            'empty' => 'There are no JBrowse locations for this feature',
          ],
        ],
      ],
    ];
  }

  if ($bundle->data_table == 'organism') {
    $field_name = 'edam__accession';
    $field_type = 'edam__accession';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Accession',
      'description' => 'Germplasm accession, breeding stock, etc.',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => TRUE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'organism_id',
        'base_table' => $bundle->data_table,
        'term_name' => 'Accession',
        'term_accession' => '2091',
        'term_vocabulary' => 'data',  // database name, not CV name
      ],
      'widget' => [
        'type' => 'edam__accession_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'inline',
          'type' => 'edam__accession_formatter',
          'settings' => [
            'title' => 'Accession',
            'empty' => 'There are no germplasm or stock entries',
          ],
        ],
      ],
    ];

  }

  return $instances;
}
