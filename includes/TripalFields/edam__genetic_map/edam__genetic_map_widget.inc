<?php

/**
 * @class
 * Purpose: Insert into featuremap linker fields, such as stock_featuremap
 */
class edam__genetic_map_widget extends ChadoFieldWidget {

  /**
   * The default label for this field.
   */
  public static $default_label = 'Genetic Map';

  /**
   * The list of field types for which this formatter is appropriate.
   */
  public static $field_types = ['edam__genetic_map'];

  /**
   * Provides the form for editing of this field.
   *
   * This function corresponds to the hook_field_widget_form()
   * function of the Drupal Field API.
   *
   * This form is diplayed when the user creates a new entity or edits an
   * existing entity.  If the field is attached to the entity then the form
   * provided by this function will be displayed.
   *
   * At a minimum, the form must have a 'value' element.  For Tripal, the
   * 'value' element of a field always corresponds to the value that is
   * presented to the end-user either directly on the page (with formatting)
   * or via web services, or some other mechanism.  However, the 'value' is
   * sometimes not enough for a field.  For example, the Tripal Chado module
   * maps fields to table columns and sometimes those columns are foreign keys
   * therefore, the Tripal Chado modules does not just use the 'value' but adds
   * additional elements to help link records via FKs.  But even in this case
   * the 'value' element must always be present in the return form and in such
   * cases it's value should be set equal to that added in the 'load' function.
   *
   * @param $widget
   * @param $form
   *   The form structure where widgets are being attached to. This might be a
   *   full form structure, or a sub-element of a larger form.
   * @param $form_state
   *   An associative array containing the current state of the form.
   * @param $langcode
   *   The language associated with $items.
   * @param $items
   *   Array of default values for this field.
   * @param $delta
   *   The order of this item in the array of subelements (0, 1, 2, etc).
   * @param $element
   *   A form element array containing basic properties for the widget:
   *   - #entity_type: The name of the entity the field is attached to.
   *   - #bundle: The name of the field bundle the field is contained in.
   *   - #field_name: The name of the field.
   *   - #language: The language the field is being edited in.
   *   - #field_parents: The 'parents' space for the field in the form. Most
   *    widgets can simply overlook this property. This identifies the location
   *    where the field values are placed within $form_state['values'], and is
   *    used to access processing information for the field through the
   *    field_form_get_state() and field_form_set_state() functions.
   *   - #columns: A list of field storage columns of the field.
   *   - #title: The sanitized element label for the field instance, ready for
   *    output.
   *   - #description: The sanitized element description for the field instance,
   *    ready for output.
   *   - #required: A Boolean indicating whether the element value is required;
   *    for required multiple value fields, only the first widget's values are
   *    required.
   *   - #delta: The order of this item in the array of subelements; see
   *    $delta above
   */
  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {
    parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);

    $field_table = $this->instance['settings']['chado_table'];
    $base_table = $this->instance['settings']['base_table'];

    // Get information about this featuremap linker table.
    $schema = chado_get_schema($field_table);
    if (!$schema) {
      drupal_set_message(t('The featuremap linker field for ' . $field_table . ' is not present in the chado database!'));
    }
    // i.e. $pkey is featuremap_featuremap_id
    $pkey = $schema['primary key'][0];

    $fkey_lcolumn = key($schema['foreign keys'][$base_table]['columns']);

    $record_id = '';

    $fk_value = (array_key_exists('#entity', $element) and $element['#entity']) ? $element['#entity']->chado_record_id : NULL;

    // If the field already has a value then it will come through the $items
    // array.  This happens when editing an existing record.
    $featuremap_id = '';
    if (count($items) > 0 and array_key_exists($delta, $items)) {
      $fk_value = tripal_get_field_item_keyval($items, $delta, 'chado-' . $field_table . '__' . $fkey_lcolumn, $fk_value);
      $record_id = tripal_get_field_item_keyval($items, $delta, 'chado-' . $field_table . '__' . $pkey, $record_id);
      $featuremap_id = tripal_get_field_item_keyval($items, $delta, 'chado-' . $field_table . '__featuremap_id', $featuremap_id);
    }

    $widget['value'] = [
      '#type' => 'value',
      '#value' => array_key_exists($delta, $items) ? $items[$delta]['value'] : '',
    ];

    // populate the list of featuremaps available for selection
    $options = [];
    $options[] = '-- Select a map --';
    $sql = "SELECT FM.featuremap_id, FM.name, FP.value AS featuremaptype FROM {featuremap} FM"
         . " LEFT JOIN {featuremapprop} FP ON FM.featuremap_id=FP.featuremap_id"
         . " LEFT JOIN {cvterm} CV ON CV.cvterm_id=FP.type_id"
         . " WHERE FP.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name='type'"
         .   " AND cv_id=(SELECT cv_id FROM {cv} WHERE name='rdfs'))"
         . " ORDER BY CV.name, FM.name";
    $results = chado_query($sql, []);
    // Iterate through the stocks
    foreach ($results as $result) {
      $options[$result->featuremap_id] = $result->name . ' (' . $result->featuremaptype . ')';
    }

    $widget['chado-' . $field_table . '__' . $pkey] = [
      '#type' => 'value',
      '#default_value' => $record_id,
    ];
    $widget['chado-' . $field_table . '__' . $fkey_lcolumn] = [
      '#type' => 'value',
      '#default_value' => $fk_value,
    ];
    $widget['chado-' . $field_table . '__featuremap_id'] = [
      '#type' => 'value',
      '#default_value' => $featuremap_id,
    ];

    $widget['value'] = [
      '#type' => 'select',
      '#title' => t('Available Maps'),
      '#description' => $element['#description'],
      '#options' => $options,
      '#default_value' => $featuremap_id,
      '#required' => $element['#required'],
      '#weight' => isset($element['#weight']) ? $element['#weight'] : 0,
      // Delta is its order in an array of subelements.
      '#delta' => $delta,
    ];
  }

  /**
   * Performs validation of the widgetForm.
   *
   * Use this validate to ensure that form values are entered correctly.
   * The 'value' key of this field must be set in the $form_state['values']
   * array anytime data is entered by the user.  It may be the case that there
   * are other fields for helping select a value. In the end those helper
   * fields must be used to set the 'value' field.
   */
  public function validate($element, $form, &$form_state, $langcode, $delta) {
    $field_name = $this->field['field_name'];
    $settings = $this->instance['settings'];
    $base_table = $settings['base_table'];
    $field_table = $settings['chado_table'];

    // Get information about this stock linker table.
    $schema = chado_get_schema($field_table);

    $lfkey_field = key($schema['foreign keys'][$base_table]['columns']);

    // Get the featuremap_id from the form state.
    $featuremap_id = $form_state['values'][$field_name]['und'][$delta]['value'];

    if ($featuremap_id) {
      $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__featuremap_id'] = $featuremap_id;
    }

    // If no map is provided then we want to set the field for deletion.
    else {
      $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__featuremap_id'] = '';
      $form_state['values'][$field_name]['und'][$delta]['value'] = 'delete me';
      $form_state['values'][$field_name]['und'][$delta]['chado-' . $field_table . '__' . $lfkey_field] = '';
      // unset($form_state['values'][$field_name]);
    }
  }

}
