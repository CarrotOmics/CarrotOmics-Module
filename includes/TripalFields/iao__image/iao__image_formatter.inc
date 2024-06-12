<?php

class iao__image_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Image File';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = ['iao__image'];

  /**
   *
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
//todo: settings appears to be empty here
    $settings = $display['settings'];
    $rows = [];
    $headers = [($settings['title'] ?? 'Image'), 'Annotations'];

    foreach ($items as $delta => $item) {
      if (empty($item['value'])) {
        continue;
      }
      // the only required item is the image itself
      // todo: this path might be stored somewhere, find out where
      // otherwise make it a settings variable
      $image_uri = '/sites/default/files/bulk_data/carrotomics/germplasm/image/' . $item['value']['uri'];
      // thumbnail of image
      $image_tn = '/sites/default/files/bulk_data/carrotomics/germplasm/icon/' . $item['value']['uri'];

      // All annotations, if present, will be converted to a single string
      $annotationstring = '';
      $annotations = $item['value']['annotations'];

      // sort by keys, case insensitive
      ksort($annotations, SORT_STRING | SORT_FLAG_CASE);

      foreach ($annotations as $key => $value) {
        // skip uri annotations
        if (preg_match('/^#/', $key)) {
          continue;
        }
        // remove CV name from label, it is just clutter for
        // the end user, but would be shown in web services
        $term = preg_replace('/^.*:/', '', $key);
        // replace underscores with spaces for display
        $term = preg_replace('/_/', ' ', $term);

        // there will be a corresponding #xxx_uri key if a link is present
        $uri_key = '#' . $key . '_uri';
        if (array_key_exists($uri_key, $annotations)) {
          $uri = $annotations[$uri_key];
          $annotationstring .= '<li><strong>' . $term . '</strong>: <a href="'
              . $uri . '">' . $value . '</a></li>';
        }
        else {
          $annotationstring .= '<li><strong>' . $term . '</strong>: ' . $value . '</li>';
        }
      }

      // todo: there's probably a function I should call here l() ?
      $image = '<a href="' . $image_uri . '"><img src="' . $image_tn . '" border="0"></a>';
      $rows[] = [
        $image,
        $annotationstring
      ];
    }

    // Build the pager
    $items_per_page = array_key_exists('items_per_page', $this->instance['settings']) ? $this->instance['settings']['items_per_page'] : 10;
    $total_records = count($rows);
    $total_pages = (int) ($total_records / $items_per_page) + 1;
    $pelement = 0;
    $current_page = pager_default_initialize($total_records, $items_per_page, $pelement);
    $pager = theme('pager', [
      'tags' => [],
      'element' => $pelement,
      'parameters' => [],
      'quantity' => $total_pages,
    ]);
    $pager = $this->ajaxifyPager($pager, $entity);
    $page_items = array_chunk($rows, $items_per_page);

    $caption = '';
    if ($total_records == 1) {
      $caption = 'There is ' . count($rows) . ' image.';
    }
    else {
      $caption = 'There are ' . count($rows) . ' images.';
    }

    $content = theme_table([
      'header' => $headers,
      'rows' => count($rows) > 0 ? $page_items[$current_page] : [],
      'attributes' => [
        'class' => 'tripal-data-table',
      ],
      'sticky' => FALSE,
      'caption' => $caption,
      'colgroups' => [],
      'empty' => $settings['empty'] ?? 'There are no images for this accession',
    ]);

    $element[0] = [
      '#type' => 'markup',
      '#markup' => $content . $pager,
    ];
  }



  public function settingsForm($view_mode, $form, &$form_state) {
    $display = $this->instance['display'][$view_mode];
    $settings = $display['settings'];
    $element = [];
    $element['title'] = [
      '#type' => 'textfield',
      '#title' => 'Table Header',
      '#default_value' => array_key_exists('title', $settings) ? $settings['title'] : 'Images',
    ];
    $element['empty'] = [
      '#type' => 'textfield',
      '#title' => 'Empty text',
      '#default_value' => array_key_exists('empty', $settings) ? $settings['empty'] : 'There are no images for this accession',
    ];

    return $element;
  }



  public function settingsSummary($view_mode) {
  $display = $this->instance['display'][$view_mode];
  $settings = $display['settings'];
  $summary = t('Title: @title<br>Empty: @empty',
      [
        '@title' => array_key_exists('title', $settings)?$settings['title']:'Images',
        '@empty' => array_key_exists('empty', $settings)?$settings['empty']:'There are no images for this accession'
      ]);

  return $summary;
  }
}
