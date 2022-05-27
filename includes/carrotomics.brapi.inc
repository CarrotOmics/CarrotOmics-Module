<?php
/**
 * @file
 * Site-specific customization for the tripal_brapi module.
 * Here we implement callbacks to modify the BrAPI configuration.
 */

/**
 * Implements hook_brapi_calls_alter().
 *
 * @param &$brapi_calls - array returned by brapi_get_calls(), passed by reference
 *
 */
function carrotomics_brapi_calls_alter(&$brapi_calls) {
  $maps = &$brapi_calls["maps"];
  $maps["callback versions"]["1.2"] = "carrotomics_brapi_v1_maps_12_json";

  $images = &$brapi_calls["images"];
  $images["callback versions"]["1.3"] = "carrotomics_brapi_v1_images_13_json";
}

/**
 * Implements hook_brapi_data_mapping_alter().
 *
 * @param &$brapi_data_mapping - array returned by brapi_get_data_mapping(), passed by reference
 *
 */
function carrotomics_brapi_data_mapping_alter(&$brapi_data_mapping) {
  $cv_settings = brapi_get_cv_settings();

  // Look up site cvterm_id values
  [$error, $pubdate_id] = carrotomics_lookup_cvterm_id('tripal_pub', 'Publication Date');
  [$error, $rdfstype_id] = carrotomics_lookup_cvterm_id('rdfs', 'type');
  [$error, $nmarkers_id] = carrotomics_lookup_cvterm_id('local', 'Number of markers');
  [$error, $nlgroups_id] = carrotomics_lookup_cvterm_id('local', 'Number of linkage groups');
  [$error, $sampleof_id] = carrotomics_lookup_cvterm_id('MAIN', 'sample_of');
  [$error, $legend_id] = carrotomics_lookup_cvterm_id('MAIN', 'legend');

  // map full definition.
  $brapi_data_mapping['map'] = [
    'table' => 'featuremap',
    'identifier' => 'featuremap_id',
    'accession' => 'name',  // Not displayed, but required, and this becomes the sort key
    'fields' => [
      'name' => 'name',
      'mapDbId' => 'featuremap_id',
      'comments' => 'description',
      'publishedDate' => [
        'table' => 'featuremapprop',
        'foreign_key' => 'featuremap_id',
        'column' => 'value',
        'selector' => [
          'type_id' => $pubdate_id,  // "tripal_pub:Publication Date"
        ],
        'field_type' => 'string',
      ],  // We don't have this
      'species' => function ($data_type, $map, $field_name, $op, $values = NULL) {
        $taxon = NULL;
        switch ($op) {
          case NULL:
            $taxon = ['read' => TRUE];
            break;

          case 'read':
            $taxon = carrotomics_brapi_get_taxon_by_id($map->featuremap_id, 'featuremap_id', 'featuremap_organism');
            break;

          default:
        }
        return $taxon;
      },
      'type' => [
        'table' => 'featuremapprop',
        'column' => 'value',
        'foreign_key' => 'featuremap_id',
        'selector' => [
          'type_id' => $rdfstype_id,  // "rdfs:type"
        ],
        'field_type' => 'string',
      ],
      'unit' => [
        'table' => 'cvterm',
        'foreign_key' => 'cvterm_id',
        'object_key' => 'unittype_id',
        'column' => 'name',
        'selector' => [],
        'field_type' => 'string',
      ],
      'markerCount' => [
        'table' => 'featuremapprop',
        'foreign_key' => 'featuremap_id',
        'column' => 'value',
        'selector' => [
          'type_id' => $nmarkers_id,  // "local:Number of markers"
        ],
        'field_type' => 'string',
      ],
      'linkageGroupCount' => [
        'table' => 'featuremapprop',
        'foreign_key' => 'featuremap_id',
        'column' => 'value',
        'selector' => [
          'type_id' => $nlgroups_id,  // "local:Number of linkage groups"
        ],
        'field_type' => 'string',
      ],
    ],
  ];



  // germplasm reconfiguration
  $brapi_data_mapping['germplasm']['fields']['germplasmName'] = 'name';
  $brapi_data_mapping['germplasm']['fields']['accessionNumber'] = 'uniquename';



  // sample reconfiguration. Originally only sampleDbID and sampleType worked.
  $brapi_data_mapping['sample']['fields'] = [
    // Table fields.
    'sampleDbId' => 'stock_id',

    // Aliases.
    // Table joins.
    'experimentId' => [
      'table' => 'stock_analysis',
      'column' => 'analysis_id',
      'selector' => [],
      'field_type' => 'string',
    ],
    'notes' => 'description',
    'plantDbId' => [
      'table' => 'stockprop',
      'column' => 'value',
      'selector' => [
        'type_id' => $cv_settings['PlantDbId'],
      ],
      'field_type' => 'string',
    ],
    'sampleType' => [
      'table' => 'cvterm',
      'column' => 'name',
      'object_key' => 'type_id',
      'foreign_key' => 'cvterm_id',
      'selector' => [],
      'field_type' => 'string',
    ],
    'sourceStockId' => [
      'table' => 'stock_relationship',
      'column' => 'object_id',
      'foreign_key' => 'subject_id',
      'selector' => [
        'type_id' => $cv_settings['SampleOf'],
      ],
      'field_type' => 'int',
    ],
    'tissueType' => [
      'table' => 'stock_cvterm',
      'column' => 'cvterm_id',
      'selector' => [
        'cvterm_id' => $cv_settings['TypeOfTissue'],
        'is_not' => FALSE,
      ],
      'field_type' => 'cvterm',
    ],
    'germplasmDbId' => [
      'table' => 'stock_relationship',
      'object_key' => 'stock_id',
      'foreign_key' => 'subject_id',
      'column' => 'object_id',
      'selector' => [
        'type_id' => $sampleof_id,  // "MAIN:sample_of"
      ],
      'field_type' => 'int',
    ],
    // Function fields.
    'observationUnitDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) {
      $observation = NULL;
      switch ($op) {
        case NULL:
          $observation = ['read' => TRUE];
          break;

        case 'read':
          $observation = NULL;

        default:
      }
      return $observation;
    },
    'plateDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
      $plate_db_id = NULL;
      switch ($op) {
        case NULL:
          $plate_db_id = ['read' => TRUE];
          break;

        case 'read':
          $experiment_id = brapi_get_field(
            $data_type,
            $stock,
            'experimentId'
          );
          if ($experimentprop = chado_generate_var(
            'nd_experimentprop',
            [
              'nd_experiment_id' => $experiment_id,
              'type_id' => $cv_settings['PlateDbId'],
            ]
          )) {
            $plate_db_id = $experimentprop->value;
          }

        default:
      }
      return $plate_db_id;
    },
    'plateIndex' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
      $plate_index = NULL;
      switch ($op) {
        case NULL:
          $plate_index = ['read' => TRUE];
          break;

        case 'read':
          $experiment_id = brapi_get_field(
            $data_type,
            $stock,
            'experimentId'
          );
          if ($experimentprop = chado_generate_var(
            'nd_experimentprop',
            [
              'nd_experiment_id' => $experiment_id,
              'type_id' => $cv_settings['PlateIndex'],
            ]
          )) {
            $plate_index = $experimentprop->value;
          }

        default:
      }
      return $plate_index;
    },
    'plotDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
      $plot_db_id = NULL;
      switch ($op) {
        case NULL:
          $plot_db_id = ['read' => TRUE];
          break;

        case 'read':
          $experiment_id = brapi_get_field(
            $data_type,
            $stock,
            'experimentId'
          );
          if ($experimentprop = chado_generate_var(
            'nd_experimentprop',
            [
              'nd_experiment_id' => $experiment_id,
              'type_id' => $cv_settings['PlotDbId'],
            ]
          )) {
            $plot_db_id = $experimentprop->value;
          }

        default:
      }
      return $plot_db_id;
    },
    'sampleTimestamp' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
      $sample_timestamp = NULL;
      switch ($op) {
        case NULL:
          $sample_timestamp = ['read' => TRUE];
          break;

        case 'read':
          $experiment_id = brapi_get_field(
            $data_type,
            $stock,
            'experimentId'
          );
          $experimentprop = chado_generate_var(
            'nd_experimentprop',
            [
              'nd_experiment_id' => $experiment_id,
              'type_id' => $cv_settings['SampleTimestamp'],
            ]
          );
          if ($experimentprop && $experimentprop->value) {
            $sample_timestamp = brapi_parse_date($experimentprop->value);
          }

        default:
      }
      return $sample_timestamp;
    },
    'studyDbId' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
      $study_db_id = NULL;
      switch ($op) {
        case NULL:
          $study_db_id = ['read' => TRUE];
          break;

        case 'read':
          $experiment_id = brapi_get_field(
            $data_type,
            $stock,
            'experimentId'
          );
          if ($experimentprop = chado_generate_var(
            'nd_experimentprop',
            [
              'nd_experiment_id' => $experiment_id,
              'type_id' => $cv_settings['Study'],
            ]
          )) {
            $study_db_id = $experimentprop->value;
          }

        default:
      }
      return $study_db_id;
    },
    'takenBy' => function ($data_type, $stock, $field_name, $op, $values = NULL) use ($cv_settings) {
      $contact = NULL;
      switch ($op) {
        case NULL:
          $contact = ['read' => TRUE];
          break;

        case 'read':
          $experiment_id = brapi_get_field(
            $data_type,
            $stock,
            'experimentId'
          );
          if ($experiment = chado_generate_var(
            'nd_experiment',
            [
              'nd_experiment_id' => $experiment_id,
            ]
          )) {
            $experiment = chado_expand_var($experiment, 'table', 'nd_experiment_contact');
            $contact = $experiment->nd_experiment_contact->contact_id->name;
          }

        default:
      }
      return $contact;
    },
  ];



  // image full definition v1.3
  $brapi_data_mapping['image'] = [
    'table' => 'eimage',
    'identifier' => 'eimage_id',
    'accession' => 'image_uri',
    'fields' => [
      // Table fields.
      'imageDbId' => 'eimage_id',
      'imageName' => 'image_uri',
      'observationUnitDbId' => [],
      'observationDbId' => [],
      'descriptiveOntologyTerm' => [
        'table' => 'eimageprop',
        'foreign_key' => 'eimage_id',
        'column' => 'value',
        'selector' => [
          'type_id' => $legend_id,  // "MAIN:legend"
        ],
        'field_type' => 'string',
      ],
      'imageURL' => function ($data_type, $image, $field_name, $op, $values = NULL) {
        $url = NULL;
        switch ($op) {
          case NULL:
            $url = ['read' => TRUE];
            break;

          case 'read':
            $sql = "SELECT I.image_uri FROM {eimage} I"
                 . " WHERE I.eimage_id=:image_id";
            $args = [':image_id' => $image->eimage_id];
            $url = chado_query($sql, $args)->fetchField();
            $url = 'https://carrotomics.org/sites/default/files/bulk_data/carrotomics/germplasm/image/' . $url;
            break;

          default:
        }
        return $url;
      },
    ],
  ];

}

/**
 * Outer callback for linkage maps.
 */
function carrotomics_brapi_v1_maps_12_json($map_id = NULL) {
  $actions = [
    'read'   => 'carrotomics_brapi_v1_read_maps_12_json',
    'list'   => 'carrotomics_brapi_v1_maps_search_12_json',
  ];
  return brapi_process_crud('maps/{mapDbId}', $actions, 'map', $map_id);
}

/**
 * Callback for linkage maps to ...
 */
function carrotomics_brapi_v1_read_maps_12_json($map_id = NULL) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = [];
  $maps_data = carrotomics_brapi_get_maps_details_12($map_id);
  $data = ['result' => $maps_data];
  brapi_aggregate_call($data, $metadata, $debug_data);

  return [
    $data,
    $metadata,
    $debug_data,
  ];
}

/**
 * Callback for linkage maps to retrieve list of maps.
 */
function carrotomics_brapi_v1_maps_search_12_json() {
  $parameters = [
    'get_object_details' => 'carrotomics_brapi_get_maps_details_12',
  ];
  return brapi_v1_object_search_json('map', $parameters);
}

/**
 * Callback for linkage maps to get details for a map.
 */
function carrotomics_brapi_get_maps_details_12($map_id) {
  $maps_data = [];

  $fields = [
    'name', 'mapDbId', 'publishedDate', 'species',
    'type', 'unit', 'markerCount', 'linkageGroupCount',
    'comments',
  ];

  // description may be long so may need to be expanded
  $map_id = chado_expand_var($map_id, 'field', 'featuremap.description');

  foreach ($fields as $field_name) {
    $maps_data[$field_name] =
      brapi_get_field('map', $map_id, $field_name);
  }
  return $maps_data;
}

/**
 * Return the scientific name of the organism from a database_id.
 */
function carrotomics_brapi_get_taxon_by_id($key_id, $fkey_name, $linker_table) {
  $sql = "SELECT organism_id FROM {".$linker_table."} WHERE $fkey_name=:fkey_name";
  $args = [':fkey_name' => $fkey_name];
  $organism_id = chado_query($sql, $args)->fetchField();
  $taxon = chado_get_organism_scientific_name(chado_get_organism(['organism_id' => $organism_id], []));
  return $taxon;
}

/**
 * Outer callback for images.
 */
function carrotomics_brapi_v1_images_13_json($image_id = NULL) {
  $actions = [
    'read'   => 'carrotomics_brapi_v1_read_images_13_json',
    'list'   => 'carrotomics_brapi_v1_images_search_13_json',
  ];
  return brapi_process_crud('images/{imageDbId}', $actions, 'image', $image_id);
}

/**
 * Callback for images to ...
 */
function carrotomics_brapi_v1_read_images_13_json($image_id = NULL) {
  $metadata = brapi_prepare_metadata(1);
  $debug_data = [];
  $images_data = carrotomics_brapi_get_images_details_13($image_id);
  $data = ['result' => $images_data];
  brapi_aggregate_call($data, $metadata, $debug_data);

  return [
    $data,
    $metadata,
    $debug_data,
  ];
}

/**
 * Callback for images to retrieve list of images.
 */
function carrotomics_brapi_v1_images_search_13_json() {
  $parameters = [
    'get_object_details' => 'carrotomics_brapi_get_images_details_13',
  ];
  return brapi_v1_object_search_json('image', $parameters);
}

/**
 * Callback for images to get details for an image.
 */
function carrotomics_brapi_get_images_details_13($image_id) {
  $image_data = [];

  $fields = [
    'imageName', 'imageDbId', 'imageURL', 'descriptiveOntologyTerm',
    'observationUnitDbId', 'observationDbId',
  ];

  foreach ($fields as $field_name) {
    $image_data[$field_name] =
      brapi_get_field('image', $image_id, $field_name);
  }
  return $image_data;
}

