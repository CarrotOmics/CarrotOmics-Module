<?php

class local__analysis_project_widget extends ChadoFieldWidget {

  // The default lable for this field.
  public static $default_label = 'CarrotOmics Entity';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = ['local__analysis_project'];

  /**
   *
   * @see TripalFieldWidget::form()
   */
  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {

    $field_name = $this->field['field_name'];
    $field_table = $this->instance['settings']['chado_table'];
    $linker_field = 'chado-' . $field_table . '__project_id';

    // The value presented to the user via load.
    // If $items['delta']['value'] is set then we are updating and already have this
    // information. As such, simply save it again.
    $widget['value'] = array(
      '#type' => 'value',
      '#value' => array_key_exists($delta, $items) ? $items[$delta]['value'] : '',
    );

    // Pull out the value previously saved to be used as the default.
    $project_id = 0;
    if (count($items) > 0 and array_key_exists($linker_field, $items[0])) {
      $project_id = $items[0][$linker_field];
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
