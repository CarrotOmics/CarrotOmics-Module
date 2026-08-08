<?php

namespace Drupal\carrotomics\Plugin\Field\FieldFormatter;

use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Link;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\Core\Url;
use Drupal\tripal\TripalField\Attribute\TripalFieldFormatter;
use Drupal\tripal_chado\TripalField\ChadoFormatterBase;

/**
 * A formatter for geographic position coordinates.
 */
#[TripalFieldFormatter(
  id: 'chado_nd_geolocation_formatter',
  label: new TranslatableMarkup('Geographic position'),
  description: new TranslatableMarkup('Geographic position coordinates: latitude, longitude, and altitude.'),
  field_types: [
    'chado_nd_geolocation',
  ],
  valid_tokens: [
    '[nd_geo_lat]',
    '[nd_geo_lon]',
    '[nd_geo_alt]',
    '[nd_geodetic_datum]',
    '[nd_geo_desc]',
    '[linker_type]',
    '[nd_exp_type]',
  ],
)]
class ChadoNdGeolocationFormatter extends ChadoFormatterBase {

  /**
   * {@inheritdoc}
   */
  public static function defaultSettings() {
    $settings = parent::defaultSettings();
    $settings['token_string'] = 'Latitude: [nd_geo_lat], Longitude: [nd_geo_lon], Altitude: [nd_geo_alt]m<br>[nd_geo_desc]';
    $settings['decimal_places'] = 6;
    $settings['decimal_places_altitude'] = 0;
    // Supported styles are 'decimal' and 'dms' for Deg°Min'Sec".
    $settings['output_style'] = 'decimal';
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
    $decimal_places = $this->getSetting('decimal_places') ?? 6;
    $decimal_places_altitude = $this->getSetting('decimal_places_altitude') ?? 0;
    $output_style = $this->getSetting('output_style') ?? 'decimal';
    $lookup_manager = \Drupal::service('tripal.tripal_entity.lookup');

    foreach ($items as $delta => $item) {
      $raw_latitude = $item->get('nd_geo_lat')->getString();
      $raw_longitude = $item->get('nd_geo_lon')->getString();
      $raw_altitude = $item->get('nd_geo_alt')->getString();

      if ($output_style == 'dms') {
        $latitude = $this->decimalToDMSString($raw_latitude, $decimal_places, 'lat');
        $longitude = $this->decimalToDMSString($raw_longitude, $decimal_places, 'lon');
      }
      else {
        $latitude = sprintf('%0.' . $decimal_places . 'f', $raw_latitude);
        $longitude = sprintf('%0.' . $decimal_places . 'f', $raw_longitude);
      }
      $altitude = sprintf('%0.' . $decimal_places_altitude . 'f', $raw_altitude);
      $values = [
        'entity_id' => $item->get('entity_id')->getString(),
        'nd_geo_lat' => $latitude,
        'nd_geo_lon' => $longitude,
        'nd_geo_alt' => $altitude,
        'nd_geo_desc' => $item->get('nd_geo_desc')->getString(),
        'nd_geodetic_datum' => $item->get('nd_geodetic_datum')->getString(),
        'linker_type' => $item->get('linker_type')->getString(),
        'nd_exp_type' => $item->get('nd_exp_type')->getString(),
      ];

      // Substitute values in token string to generate displayed string.
      $displayed_string = $token_string;
      foreach ($values as $key => $value) {
        $displayed_string = preg_replace("/\[$key\]/", $value, $displayed_string);
      }
      $displayed_string = trim($displayed_string);

      // Create a clickable link to the corresponding entity when one exists.
      $renderable_item = $lookup_manager->getRenderableItem($displayed_string, $values['entity_id']);

      if (strlen($raw_latitude) && strlen($raw_longitude)) {
        // Bounding box coordinates for the mini-map.
        $lat_margin = 10;
        $lon_margin = 12;
        $lat1 = $raw_latitude - $lat_margin;
        $lon1 = $raw_longitude - $lon_margin;
        $lat2 = $raw_latitude + $lat_margin;
        $lon2 = $raw_longitude + $lon_margin;

        $iframe = '<iframe width="300" height="250" src="https://www.openstreetmap.org/export/embed?bbox='
        . $lon1 . '%2C' . $lat1 . '%2C' . $lon2 . '%2C' . $lat2 . '&amp;layer=mapnik&amp;marker='
        . $raw_latitude . '%2C' . $raw_longitude . '" style="border: 1px solid black"></iframe>';

        $zoom = 6;
        $url = 'https://www.openstreetmap.org/?mlat=' . $raw_latitude . '&mlon=' . $raw_longitude . '#map=' . $zoom . '/' . $raw_latitude . '/' . $raw_longitude;
        $url_object = Url::fromUri($url);
        $url_object->setOptions(['attributes' => ['target' => '_blank']]);
        $link = Link::fromTextAndUrl('View Larger Map', $url_object);

        // Adds the mini-map iframe.
        $renderable_item['#markup'] .= "<br>\n" . $iframe;

        // Adds the link to view a larger map.
        $renderable_item['#markup'] .= "<br>\n" . $link->toString();
        $renderable_item['#allowed_tags'] = ['a', 'br', 'iframe'];
      }
      $list[$delta] = $renderable_item;
    }

    // Will convert $list to a markup list if there is more than one item.
    $elements = $this->createListMarkup($list);
    return $elements;
  }

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array $form, FormStateInterface $form_state) {
    $form = parent::settingsForm($form, $form_state);

    $form['output_style'] = [
      '#title' => $this->t('Style for coordinates'),
      '#description' => $this->t('Selects the style used for latitude and longitude'),
      '#type' => 'select',
      '#options' => [
        'decimal' => $this->t('Decimal'),
        'dms' => $this->t('Degrees Minutes Seconds'),
      ],
      '#default_value' => $this->getSetting('output_style'),
      '#required' => FALSE,
    ];
    $form['decimal_places'] = [
      '#title' => $this->t('Decimal places'),
      '#description' => $this->t('Sets the number of decimal places displayed for latitude and longitude'),
      '#type' => 'number',
      '#min' => 0,
      '#max' => 16,
      '#default_value' => $this->getSetting('decimal_places'),
      '#required' => FALSE,
    ];
    $form['decimal_places_altitude'] = [
      '#title' => $this->t('Decimal places for Altitude'),
      '#description' => $this->t('Sets the number of decimal places displayed for altitude'),
      '#type' => 'number',
      '#min' => 0,
      '#max' => 16,
      '#default_value' => $this->getSetting('decimal_places_altitude'),
      '#required' => FALSE,
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function settingsSummary() {
    $summary = parent::settingsSummary();
    $decimal_places = $this->getSetting('decimal_places');
    $output_style = $this->getSetting('output_style');
    $summary[] = $this->t('Style: @output_style', ['@output_style' => $output_style]);
    $summary[] = $this->t('Places: @decimal_places', ['@decimal_places' => $decimal_places]);
    return $summary;
  }

  /**
   * Converts decimal degrees to degrees, minutes, seconds.
   *
   * @param float $value
   *   Latitude or longitude as a decimal value.
   * @param int $decimal_places
   *   Number of decimal places to round seconds to.
   * @param string $type
   *   Either 'lat' or 'lon'.
   *
   * @return string
   *   The coordinate string in degrees, minutes, seconds.
   */
  protected function decimalToDMSString(float $value, int $decimal_places, string $type): string {

    // Determine hemisphere direction.
    if ($type === 'lat') {
      $direction = ($value >= 0) ? 'N' : 'S';
    }
    else {
      $direction = ($value >= 0) ? 'E' : 'W';
    }

    // Divide into components.
    $absolute_value = abs($value);
    $degrees = floor($absolute_value);
    $fractional_minutes = ($absolute_value - $degrees) * 60;
    $minutes = floor($fractional_minutes);
    $seconds = round(($fractional_minutes - $minutes) * 60, $decimal_places);

    // Handle rounding overflows.
    if ($seconds >= 60) {
      $seconds -= 60;
      $minutes += 1;
    }
    if ($minutes >= 60) {
      $minutes -= 60;
      $degrees += 1;
    }

    // Pad with zeros to make sure minutes and seconds are two digits.
    $pad_min = sprintf('%02d', $minutes);
    // E.g. %05.2f pads seconds to 5 total characters (00.00).
    $pad_sec = sprintf('%0' . ($decimal_places ? ($decimal_places + 3) : 2) . '.' . $decimal_places . 'f', $seconds);

    // Combine all the components.
    $formatted = $degrees . '°' . $pad_min . "'" . $pad_sec . '"' . $direction;

    return $formatted;
  }

}
