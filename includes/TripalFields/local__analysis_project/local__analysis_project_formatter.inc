<?php

class local__analysis_project_formatter extends ChadoFieldFormatter {

  // The default lable for this field.
  public static $default_label = 'Related project or analysis';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = ['local__analysis_project'];

  /**
   *
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
//@@@todo: settings appears to be empty here
    $settings = $display['settings'];
    $rows = [];
//    $headers = [($settings['title'] ?? 'Accession')];
    $headers = ['Project', 'Project Type'];

    foreach ($items as $delta => $item) {
      if (empty($item['value'])) {
        continue;
      }
      // link to the entity
      $label = $item['value']['rdfs:label'];
      $name = $item['value']['schema:name'];
      if (array_key_exists('entity', $item['value'])) {
        list($e_type, $e_id) = explode(':', $item['value']['entity']);
        $content = l($name, '/' . $e_type . '/' . $e_id, []);
      }

      if ($content) {
        $rows[] = [
          $content,
          $label,
        ];
      }
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
      $caption = 'There is ' . count($rows) . ' analysis.';
    }
    else {
      $caption = 'There are ' . count($rows) . ' analyses.';
    }
    // for now I will not use a caption
    $caption = '';

    $content = theme_table([
      'header' => $headers,
      'rows' => count($rows) > 0 ? $page_items[$current_page] : [],
      'attributes' => [
        'class' => 'tripal-data-table',
      ],
      'sticky' => FALSE,
      'caption' => $caption,
      'colgroups' => [],
      'empty' => $settings['empty'] ?? 'There is no linked information',
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
      '#default_value' => array_key_exists('title', $settings) ? $settings['title'] : 'CarrotOmics Information',
    ];
    $element['empty'] = [
      '#type' => 'textfield',
      '#title' => 'Empty text',
      '#default_value' => array_key_exists('empty', $settings) ? $settings['empty'] : 'There is no linked information',
    ];

    return $element;
  }



  public function settingsSummary($view_mode) {
  $display = $this->instance['display'][$view_mode];
  $settings = $display['settings'];
  $summary = t('Title: @title<br>Empty: @empty',
      [
        '@title' => array_key_exists('title', $settings)?$settings['title']:'CarrotOmics Information',
        '@empty' => array_key_exists('empty', $settings)?$settings['empty']:'There is no linked information'
      ]);

  return $summary;
  }
}
