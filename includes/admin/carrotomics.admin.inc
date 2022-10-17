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

  // Validation Functions tab
  $items[] = "Find Unpublished Content";
  $items[] = "Validate CarrotOmics accessions";
  $items[] = "Validate public:// chado.fileloc entries";

  // Publication Functions tab
  $items[] = "Validate publications";
  $items[] = "Update publication Unique Local Identifier";
  $items[] = "Generate publication URLs";
  $items[] = "Make abstract URLs clickable";
  $items[] = "Share pub and pubprop information";
  $items[] = "Get maximum pub_id value";
  $items[] = "Find Duplicate Publications";
  $items[] = "Find Unlinked Errata";

  // Data Export Functions tab
  $items[] = "GBS Marker Locations";
  $items[] = "Update stock_featuremap table";
  $items[] = "Export gff3 of all mapped markers";

  // SQL Functions tab
  $items[] = "Clean MD5 Checksums";
  $items[] = "Clean Null taxids";
  $items[] = "Populate organism abbreviations";
  $items[] = "Populate feature residues Length and Checksum";
  $items[] = "Generate QTL Abbreviations";
  $items[] = "Make marker_locus match genetic_marker";
  $items[] = "Generate stock_analysis links";
  $items[] = "Generate map properties";
  $items[] = "Assign organisms to phylonodes";

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
