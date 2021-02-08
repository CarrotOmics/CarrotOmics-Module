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
//file_put_contents( "/tmp/log.carrotomics", "CP5a view()\n", FILE_APPEND ); //@@@
//todo: settings appears to be empty here
    $settings = $display['settings'];
//ob_start(); var_dump($display); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP5b load() display = ".$astring."\n", FILE_APPEND ); //@@@
//ob_start(); var_dump($settings); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP5c load() settings = ".$astring."\n", FILE_APPEND ); //@@@
    $rows = [];
    $headers = [($settings['title'] ?? 'Image'), 'Annotations'];

    foreach ($items as $delta => $item) {
//ob_start(); var_dump($delta); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP5d load() delta = ".$astring."\n", FILE_APPEND ); //@@@
//ob_start(); var_dump($item); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP5e load() item = ".$astring."\n", FILE_APPEND ); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP5f load() add image\n", FILE_APPEND ); //@@@
      if (empty($item['value'])) {
        continue;
      }
      // the only required item is the image itself
      // todo: this path might be stored somewhere, find out where
      // otherwise make it a settings variable
      $uri = '/sites/default/files/bulk_data/carrotomics/germplasm/image/' . $item['value']['uri'];
      // thumbnail of image
      $tn = '/sites/default/files/bulk_data/carrotomics/germplasm/icon/' . $item['value']['uri'];

//https://test.carrotomics.org/sites/default/files/k11611-1i.tn_.jpg

      // All annotations, if present, will be converted to a single string
      $annotationstring = '';
      $annotations = $item['value']['annotations'];

      // add the file name without path as one of the annotations
      $annotations['file_name'] = preg_replace('/^.*\//', '', $item['value']['uri']);

      // sort by keys, case insensitive
      ksort($annotations, SORT_STRING | SORT_FLAG_CASE);

      foreach ($annotations as $key => $value) {
        // remove CV name from label, it is just clutter for the end user
        $term = preg_replace('/^.*:/', '', $key);
        // replace underscores with spaces for display
        $term = preg_replace('/_/', ' ', $term);
        // todo: project_id should link to project. For now mask it
        if ($term != 'project id') {
          $annotationstring .= '<li><strong>' . $term . '</strong>: ' . $value . '</li>';
        }
      }

      // todo: there's probably a function I should call here l() ?
      $image = '<a href="'.$uri.'"><img src="'.$tn.'" border="0"></a>';
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
//file_put_contents( "/tmp/log.carrotomics", "CP5f load() items_per_page=$items_per_page total_records=$total_records total_pages=$total_pages pelement=$pelement current_page=$current_page \n", FILE_APPEND ); //@@@

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
//file_put_contents( "/tmp/log.carrotomics", "CP6a settingsForm()\n", FILE_APPEND ); //@@@
    $display = $this->instance['display'][$view_mode];
//ob_start(); var_dump($display); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP6b settingsForm() display = ".$astring."\n", FILE_APPEND ); //@@@
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

//ob_start(); var_dump($element); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP6c settingsForm() element = ".$astring."\n", FILE_APPEND ); //@@@
    return $element;
  }



  public function settingsSummary($view_mode) {
//file_put_contents( "/tmp/log.carrotomics", "CP7a settingsSummary()\n", FILE_APPEND ); //@@@
  $display = $this->instance['display'][$view_mode];
  $settings = $display['settings'];
  $summary = t('Title: @title<br>Empty: @empty',
      [
        '@title' => array_key_exists('title', $settings)?$settings['title']:'Images',
        '@empty' => array_key_exists('empty', $settings)?$settings['empty']:'There are no images for this accession'
      ]);

//ob_start(); var_dump($summary); $astring = ob_get_clean(); //@@@
//file_put_contents( "/tmp/log.carrotomics", "CP7b settingsSummary() summary = ".$astring."\n", FILE_APPEND ); //@@@
  return $summary;
  }
}
