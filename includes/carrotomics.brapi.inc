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
}

/**
 * Implements hook_brapi_data_mapping_alter().
 *
 * @param &$brapi_data_mapping - array returned by brapi_get_data_mapping(), passed by reference
 *
 */
function carrotomics_brapi_data_mapping_alter(&$brapi_data_mapping) {
  $brapi_data_mapping["map"] = [
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
          'type_id' => 117,  // "tripal_pub:Publication Date" hardcoded for CarrotOmics
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
            $taxon = carrotomics_brapi_get_taxon_by_mapid($map->featuremap_id);
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
          'type_id' => 2962,  // "rdfs:type" hardcoded for CarrotOmics
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
          'type_id' => 64901,  // "local:Number of markers" hardcoded for CarrotOmics
        ],
        'field_type' => 'string',
      ],
      'linkageGroupCount' => [
        'table' => 'featuremapprop',
        'foreign_key' => 'featuremap_id',
        'column' => 'value',
        'selector' => [
          'type_id' => 64904,  // "local:Number of linkage groups" hardcoded for CarrotOmics
        ],
        'field_type' => 'string',
      ],
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
 * Return the scientific name of the organism from a featuremap_id.
 */
function carrotomics_brapi_get_taxon_by_mapid($map_id) {
  $sql = "SELECT organism_id FROM {featuremap_organism} WHERE featuremap_id=:featuremap_id";
  $args = [':featuremap_id' => $map_id];
  $organism_id = chado_query($sql, $args)->fetchField();
  $taxon = chado_get_organism_scientific_name(chado_get_organism(['organism_id' => $organism_id], []));
  return $taxon;
}
