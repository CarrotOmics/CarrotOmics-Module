<?php
/**
 * Configure CarrotOmics module
 *
 * @param $form
 * @param $form_state
 *
 * @ingroup carrotomics_admin
 */
function carrotomics_admin_validate_form($form, &$form_state) {

  // Set the breadcrumb
  $breadcrumb = [];
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('CarrotOmics', 'admin/carrotomics/validate');
  drupal_set_breadcrumb($breadcrumb);

  // Create the form
  $form = [];

  // Add a 'xxxxxx' button
  $form['xxx_xxx_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'xxx_xxx_btn',
    '#value'  => 'Future Use',
    '#prefix' => "<div><em>Future use, this button does nothing"
                 . ".</em></div><br />",
    '#suffix' => "<hr>",
  ];

  return $form;
}

/**
 * Validates the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_validate_form_validate($form, &$form_state) {
}

/**
 * Submits the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_validate_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;

  // Gets the trigger element.
  $triggering_element = $form_state['triggering_element']['#name'];

  if ($triggering_element == 'xxx_xxx_btn') {
    drupal_set_message( t('This button does not do anything'), 'error');
  }
  else {
    drupal_set_message("Unknown button was pressed", 'error');
  }
}
