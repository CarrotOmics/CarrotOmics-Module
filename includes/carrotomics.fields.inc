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

  // if germplasm or stock entity, then we'll attach our
  // fields to display the information
  // CO_010 = Crop Germplasm Ontology
  if ( ($term_accession == 'CO_010__0000044')     // accession
    or ($term_accession == 'CO_010__0000029')     // cultivar
    or ($term_accession == 'CO_010__0000255')     // generated germplasm
    or ($term_accession == 'CO_010__0000162') ) { // 414 inbred line
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
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => [
        'type' => 'field_chado_storage',
      ],
      'settings' => [],
    ];
  }

  // for biomaterial entity
  if ($term_accession == 'sep__00195') {
    $field_name = 'local__biomaterial_project';
    $field_type = 'local__biomaterial_project';
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
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => [
        'type' => 'field_chado_storage',
      ],
      'settings' => [],
    ];

    $field_name = 'local__analysis_project';
    $field_type = 'local__analysis_project';
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
  }

  // for organism entity
  if ($term_accession == 'OBI__0100026') {
    $field_name = 'edam__accession';
    $field_type = 'edam__accession';
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
  }

  // for project entity
  if ( ($term_accession == 'NCIT__C47885')
    or ($term_accession == 'local__Genome Project') ) {
    $field_name = 'local__project_analysis';
    $field_type = 'local__project_analysis';
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
  if (isset($bundle->data_table) AND ($bundle->data_table == 'stock')) {

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

  if (isset($bundle->data_table) AND ($bundle->data_table == 'biomaterial')) {
    $field_name = 'local__biomaterial_project';
    $field_type = 'local__biomaterial_project';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Projects and Analyses',
      'description' => 'Projects and analyses utilizing this biomaterial',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => TRUE,
        'chado_table' => 'project', // $bundle->data_table,
        'chado_column' => 'project_id',
        'base_table' => 'project', // $bundle->data_table,
        'term_name' => 'biomaterial_project',
        'term_accession' => 'biomaterial_project',
        'term_vocabulary' => 'local',  // database name, not CV name
      ],
      'widget' => [
        'type' => 'local__biomaterial_project_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'inline',
          'type' => 'local__biomaterial_project_formatter',
          'settings' => [
            'title' => 'Projects and Analyses',
            'empty' => 'There are no projects or analyses utilizing this biomaterial',
          ],
        ],
      ],
    ];
  }

  if (isset($bundle->data_table) AND ($bundle->data_table == 'analysis')) {
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

    $field_name = 'local__analysis_project';
    $field_type = 'local__analysis_project';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Related Projects',
      'description' => 'Projects related to this analysis',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => TRUE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'project_id',
        'base_table' => $bundle->data_table,
        'term_name' => 'local:analysis_project',
        'term_accession' => 'analysis_project',
        'term_vocabulary' => 'local',  // database name, not CV name
      ],
      'widget' => [
        'type' => 'local__analysis_project_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'inline',
          'type' => 'local__analysis_project_formatter',
          'settings' => [
            'title' => 'TODO8465',  // Table Header
            'empty' => 'TODO8466',  // Empty text
          ],
        ],
      ],
    ];
  }

  if (isset($bundle->data_table) AND ($bundle->data_table == 'organism')) {
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

  if (isset($bundle->data_table) AND ($bundle->data_table == 'project')) {
    $field_name = 'local__project_analysis';
    $field_type = 'local__project_analysis';
    $instances[$field_name] = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Related Analyses',
      'description' => 'Analyses related to this project',
      'required' => FALSE,
      'settings' => [
        'auto_attach' => TRUE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'project_id',
        'base_table' => $bundle->data_table,
        'term_name' => 'local:project_analysis',
        'term_accession' => 'project_analysis',
        'term_vocabulary' => 'local',  // database name, not CV name
      ],
      'widget' => [
        'type' => 'local__project_analysis_widget',
        'settings' => [],
      ],
      'display' => [
        'default' => [
          'label' => 'inline',
          'type' => 'local__project_analysis_formatter',
          'settings' => [
            'title' => 'TODO8365',  // Table Header
            'empty' => 'TODO8366',  // Empty text
          ],
        ],
      ],
    ];

  }

  return $instances;
}
