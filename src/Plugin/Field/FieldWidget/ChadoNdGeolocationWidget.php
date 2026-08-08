<?php

namespace Drupal\carrotomics\Plugin\Field\FieldWidget;

use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\StringTranslation\TranslatableMarkup;
use Drupal\tripal\TripalField\Attribute\TripalFieldWidget;
use Drupal\tripal_chado\Controller\ChadoCVTermAutocompleteController;
use Drupal\tripal_chado\TripalField\ChadoWidgetBase;

/**
 * Plugin implementation of the 'chado_nd_geolocation_widget' field widget.
 */
#[TripalFieldWidget(
  id: 'chado_nd_geolocation_widget',
  label: new TranslatableMarkup('Geographic Position'),
  description: new TranslatableMarkup('Geographic position coordinates: latitude, longitude, and altitude.'),
  field_types: [
    'chado_nd_geolocation',
  ],
)]
class ChadoNdGeolocationWidget extends ChadoWidgetBase {

  /**
   * {@inheritdoc}
   */
  public function formElement(FieldItemListInterface $items, $delta, array $element, array &$form, FormStateInterface $form_state) {

    // Cannot currently implement because of extra hop.
    return [];

    // Get the field settings.
    $field_definition = $items[$delta]->getFieldDefinition();
    $storage_settings = $field_definition->getSetting('storage_plugin_settings');
    $linker_fkey_column = $storage_settings['linker_fkey_column']
      ?? $storage_settings['base_column'] ?? 'nd_experiment_id';
    $property_definitions = $items[$delta]->getFieldDefinition()->getFieldStorageDefinition()->getPropertyDefinitions();
    $field_name = $items->getFieldDefinition()->get('field_name');

    $item_vals = $items[$delta]->getValue();
    $record_id = $item_vals['record_id'] ?? 0;
    $linker_id = $item_vals['linker_id'] ?? 0;
    $link = $item_vals['link'] ?? 0;
    $linker_type_id = $item_vals['linker_type_id'] ?? 0;
    $nd_exp_type_id = $item_vals['nd_exp_type_id'] ?? 0;
    $nd_experiment_id = $item_vals['nd_experiment_id'] ?? 0;
    $nd_geolocation_id = $item_vals['nd_geolocation_id'] ?? 0;
    $nd_geo_latitude = $item_vals['nd_geo_lat'] ?? '';
    $nd_geo_longitude = $item_vals['nd_geo_lon'] ?? '';
    $nd_geo_altitude = $item_vals['nd_geo_alt'] ?? '';
    // A common geodetic_datum is 'WGS 84'.
    $nd_geodetic_datum = $item_vals['nd_geodetic_datum'] ?? '';
    $nd_geo_description = $item_vals['nd_geo_desc'] ?? '';

    $element['record_id'] = [
      '#type' => 'value',
      '#default_value' => $record_id,
    ];
    $element['linker_id'] = [
      '#type' => 'value',
      '#default_value' => $linker_id,
    ];
    $element['link'] = [
      '#type' => 'value',
      '#default_value' => $link,
    ];
    // pass the foreign key name through the form for massageFormValues().
    $elements['linker_fkey_column'] = [
      '#type' => 'value',
      '#default_value' => $linker_fkey_column,
    ];
    // pass the field machine name through the form for massageFormValues().
    $elements['field_name'] = [
      '#type' => 'value',
      '#default_value' => $field_name,
    ];
    $element['nd_experiment_id'] = [
      '#type' => 'value',
      '#default_value' => $nd_experiment_id,
    ];
    // pass the foreign key name through the form for massageFormValues().
    $element['linker_fkey_column'] = [
      '#type' => 'value',
      '#default_value' => $linker_fkey_column,
    ];
    // pass the field machine name through the form for massageFormValues()
    $element['field_name'] = [
      '#type' => 'value',
      '#default_value' => $field_name,
    ];
    $element['nd_geolocation_id'] = [
      '#type' => 'value',
      '#default_value' => $nd_geolocation_id,
    ];

    $element['nd_geo_lat'] = [
      '#type' => 'number',
      '#step' => 'any',
      '#min' => -90,
      '#max' => 90,
      '#title' => $this->t('Latitude'),
      '#default_value' => $nd_geo_latitude,
      '#placeholder' => '',
    ];
    $element['nd_geo_lon'] = [
      '#type' => 'number',
      '#step' => 'any',
      '#min' => -180,
      '#max' => 180,
      '#title' => $this->t('Longitude'),
      '#default_value' => $nd_geo_longitude,
      '#placeholder' => '',
    ];
    $element['nd_geo_alt'] = [
      '#type' => 'number',
      '#step' => 'any',
      '#title' => $this->t('Altitude'),
      '#default_value' => $nd_geo_altitude,
      '#placeholder' => '',
    ];
    $element['nd_geo_desc'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Description'),
      '#default_value' => $nd_geo_description,
      '#placeholder' => '',
    ];
    $element['nd_geodetic_datum'] = [
      '#type' => 'textfield',
      '#maxlength' => 32,
      '#title' => $this->t('Geodetic Datum'),
      '#default_value' => $nd_geodetic_datum,
      '#placeholder' => '',
    ];

    // CV term autocomplete. This controller includes synonyms.
    $linker_term_autocomplete_default = '';
    if ($linker_type_id) {
      $cv_autocomplete = new ChadoCVTermAutocompleteController();
      $linker_term_autocomplete_default = $cv_autocomplete->formatCVterm($linker_type_id);
    }
    $element['linker_type_id'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Link Type'),
      '#required' => FALSE,
      '#default_value' => $linker_term_autocomplete_default,
      '#disabled' => FALSE,
      '#autocomplete_route_name' => 'tripal.cvterm_autocomplete',
      '#autocomplete_route_parameters' => ['count' => 10],
      '#element_validate' => [[static::class, 'validateAutocomplete']],
    ];

    $nd_experiment_term_autocomplete_default = '';
    if ($nd_exp_type_id) {
      $cv_autocomplete = new ChadoCVTermAutocompleteController();
      $nd_experiment_term_autocomplete_default = $cv_autocomplete->formatCVterm($nd_exp_type_id);
    }
    $element['nd_exp_type_id'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Experiment Type'),
      '#required' => FALSE,
      '#default_value' => $nd_experiment_term_autocomplete_default,
      '#disabled' => FALSE,
      '#autocomplete_route_name' => 'tripal.cvterm_autocomplete',
      '#autocomplete_route_parameters' => ['count' => 10],
      '#element_validate' => [[static::class, 'validateAutocomplete']],
    ];

    return $element;
  }

  /**
   * {@inheritDoc}
   */
  public function massageFormValues(array $values, array $form, FormStateInterface $form_state) {

    // Cannot currently implement because extra hop.
    return $values;

    foreach ($values as $delta => $value) {
      $new_value = $value;

      // Use autocomplete to replace the term with its cvterm_id value.
      $linker_type_id = 0;
      if ($value['linker_type_id']) {
        $cv_autocomplete = new ChadoCVTermAutocompleteController();
        $linker_type_id = $cv_autocomplete->getCVtermId($value['linker_type_id']);
      }
      else {
        $linker_type_id = 0;
      }
      $nd_exp_type_id = 0;
      if ($value['nd_exp_type_id']) {
        $cv_autocomplete = new ChadoCVTermAutocompleteController();
        $nd_exp_type_id = $cv_autocomplete->getCVtermId($value['nd_exp_type_id']);
      }
      else {
        $nd_exp_type_id = 0;
      }
      if ($value['nd_geo_lat'] && $value['nd_geo_lon']) {
        // If you add a term but no coordinates, it will just be ignored, thus
        // we don't add the term until this point, when we know there are
        // coordinates.
        $new_value['linker_type_id'] = $linker_type_id;
        $new_value['nd_exp_type_id'] = $nd_exp_type_id;
      }

      // We need at a minimum latitude and longitude values. If not,
      // consider this delta empty and remove it.
      if ($value['nd_geo_lat'] && $value['nd_geo_lon']) {
        $values[$delta] = $new_value;
      }
      else {
        unset($values[$delta]);
      }
    }

    return $values;
  }

  /**
   * Form element validation handler for the CV term field
   *
   * We permit entering a term without other fields, it will just
   * be ignored.
   *
   * @param array $element
   *   The form element being validated
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The form state of the (entire) configuration form
   */
  public static function validateAutocomplete($element, FormStateInterface $form_state) {
    $element_parents = $element['#parents'];
    $element_value = $element['#value'];
    if ($element_value != '') {
      $cv_autocomplete = new ChadoCVTermAutocompleteController();
      $cvterm_id = $cv_autocomplete->getCVtermId($element_value);
      if (!$cvterm_id) {
        $form_state->setErrorByName(implode('][', $element_parents),
            t('The Controlled Vocabulary Term "@term" is not a valid term', ['@term' => $element_value]));
      }
    }
  }

}
