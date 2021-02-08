<?php

class sio__geographic_position_widget extends ChadoFieldWidget {

  // The default lable for this field.
  public static $default_label = 'Geographic Position';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = ['sio__geographic_position'];

  /**
   *
   * @see TripalFieldWidget::form()
   */
  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {

    $field_name = $this->field['field_name'];
    $field_table = $this->instance['settings']['chado_table'];
    $linker_field = 'chado-' . $field_table . '__nd_geolocation_id';

    // The value presented to the user via load.
    // If $items['delta']['value'] is set then we are updating and already have this
    // information. As such, simply save it again.
    $widget['value'] = array(
      '#type' => 'value',
      '#value' => array_key_exists($delta, $items) ? $items[$delta]['value'] : '',
    );

    // Pull out the value previously saved to be used as the default.
    $nd_geolocation_id = 0;
    if (count($items) > 0 and array_key_exists($linker_field, $items[0])) {
      $nd_geolocation_id = $items[0][$linker_field];
    }

    parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);

  }

  /**
   *
   * @see TripalFieldWidget::submit()
   */
  public function submit($form, &$form_state, $entity_type, $entity, $langcode, $delta) {

  }
}
