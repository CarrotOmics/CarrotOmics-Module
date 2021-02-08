<?php

class sio__geographic_position_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Geographic Position';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = ['sio__geographic_position'];

  /**
   *
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    $field_name = $this->field['field_name'];
    $chado_table = $this->instance['settings']['chado_table'];

    $headers = [
      'Latitude', 'Uncertainty (±)',
      'Longitude', 'Uncertainty (±)',
      'Altitude (m)', 'Uncertainty (±)',
      'Description'
    ];
    $rows = [];
    $populated = 0;

    // obtain the values we wish to display
    foreach ($items as $item) {

      $description = $item['value']['schema:description'] ?? '';
      $latitude = $item['value']['SIO:latitude'] ?? 'unavailable';
      $longitude = $item['value']['SIO:longitude'] ?? 'unavailable';
      $altitude = $item['value']['SIO:altitude'] ?? 'unavailable';
      $latitude_dev = $item['value']['local:latitude_dev'] ?? '';
      $longitude_dev = $item['value']['local:longitude_dev'] ?? '';
      $altitude_dev = $item['value']['local:altitude_dev'] ?? '';

      // mask display of deviation values if they are zero
      $latitude_dev = $latitude_dev?$latitude_dev:'';
      $longitude_dev = $longitude_dev?$longitude_dev:'';
      $altitude_dev = $altitude_dev?$altitude_dev:'';

      $rows[] = [
        $latitude, $latitude_dev,
        $longitude, $longitude_dev,
        $altitude, $altitude_dev,
        $description
      ];
      if ( ( $latitude ) and ( $latitude !== 'unavailable' ) ) { $populated = 1; }
      if ( ( $longitude ) and ( $longitude !== 'unavailable' ) ) { $populated = 1; }
      if ( ( $altitude ) and ( $altitude !== 'unavailable' ) ) { $populated = 1; }
      if ( $description ) { $populated = 1; }
    }

    $table = [
      'header' => $headers,
      'rows' => $rows,
      'attributes' => [
        'id' => 'sio__geographic_position-table',
        'class' => 'tripal-data-table',
      ],
      'sticky' => FALSE,
      'caption' => '',
      'colgroups' => [],
      'empty' => 'There is no geographic location information.',
    ];

    // once we have our table array structure defined, we call Drupal's theme_table()
    // function to generate the table.
    if ( $populated ) {
      $content = theme_table($table);
      $element[0] = [
        '#type' => 'markup',
        '#markup' => $content,
      ];
    }
    // If we wanted an empty "Geographic Location" field displayed, an else clause could be defined here
  }



  public function settingsForm($view_mode, $form, &$form_state) {

  }



  public function settingsSummary($view_mode) {

  }
}
