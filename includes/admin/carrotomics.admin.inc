<?php

/**
 * @file
 * Administrative interface for the CarrotOmics module
 *
 * @ingroup carrotomics_admin
 */

/**
 * Admin page
 *
 */
function carrotomics_admin_page() {

  // Set the breadcrumb
  $breadcrumb = [];
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('CarrotOmics', 'admin/carrotomics');
  drupal_set_breadcrumb($breadcrumb);

  // Description.
  $items = [];
  $items[] = "Add URLs to publications";
  $description =  theme('item_list', ['items' => $items]);

  $content =<<<ADMIN_PAGE
<div><em>Administrative pages for the CarrotOmics module</em></div><br />
<div>
  <b>Functions Provided</b><br />
  $description
</div>
ADMIN_PAGE;

  return $content;
}

/**
 *
 */
function carrotomics_admin_form($form, $form_state) {
  return $form;
}

/**
 * Validate the Admin/Settings form.
 */
function carrotomics_admin_form_validate($form, &$form_state) {
}

/**
 * Submit the Admin/settings form.
 */
function carrotomics_admin_form_submit($form, $form_state) {
}
