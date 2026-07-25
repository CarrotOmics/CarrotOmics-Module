<?php

namespace Drupal\carrotomics\Plugin\Field\FieldFormatter;

use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\tripal\TripalField\Attribute\TripalFieldFormatter;
use Drupal\tripal_chado\TripalField\ChadoFormatterBase;

/**
 * The 'chado_nd_geolocation_formatter' field formatter.
 */
#[TripalFieldFormatter(
  id: 'chado_nd_geolocation_formatter',
  label: new TranslatableMarkup('Geographic position'),
  description: new TranslatableMarkup('Geographic position coordinates: latitude, longitude, and altitude.'),
  field_types: [
    'chado_nd_geolocation',
  ],
  valid_tokens: [
    '[nd_geo_latitude]',
    '[nd_geo_longitude]',
    '[nd_geo_altitude]',
    '[nd_geo_geodetic_datum]',
    '[nd_geo_description]',
    '[linker_type]',
  ],
)]
class ChadoNdGeolocationFormatter extends ChadoFormatterBase {

  /**
   * {@inheritdoc}
   */
  public static function defaultSettings() {
    $settings = parent::defaultSettings();
    // No label for type or description as they may be empty.
    $settings['token_string'] = '[linker_type] Latitude: [nd_geo_latitude], Longitude: [nd_geo_longitude], Altitude: [nd_geo_altitude] [nd_geo_description]';
    return $settings;
  }

  /**
   * {@inheritdoc}
   */
  public function viewElements(FieldItemListInterface $items, $langcode) {
    $elements = [];
    parent::viewElements($items, $langcode);
    $list = [];
    $token_string = $this->getSetting('token_string');
    $lookup_manager = \Drupal::service('tripal.tripal_entity.lookup');

    foreach ($items as $delta => $item) {
      $values = [
        'entity_id' => $item->get('entity_id')->getString(),
        'nd_geo_latitude' => $item->get('nd_geo_latitude')->getString(),
        'nd_geo_longitude' => $item->get('nd_geo_longitude')->getString(),
        'nd_geo_altitude' => $item->get('nd_geo_altitude')->getString(),
        'nd_geo_description' => $item->get('nd_geo_description')->getString(),
        'nd_geo_geodetic_datum' => $item->get('nd_geo_geodetic_datum')->getString(),
        'linker_type' => $item->get('linker_type')->getString(),
      ];

      // Substitute values in token string to generate displayed string.
      $displayed_string = $token_string;
      foreach ($values as $key => $value) {
        $displayed_string = preg_replace("/\[$key\]/", $value, $displayed_string);
      }
      $displayed_string = trim($displayed_string);

      // Create a clickable link to the corresponding entity when one exists.
      $renderable_item = $lookup_manager->getRenderableItem($displayed_string, $values['entity_id']);

      $list[$delta] = $renderable_item;
    }

    // Will convert $list to a markup list if there is more than one item.
    $elements = $this->createListMarkup($list);
    return $elements;
  }

}
