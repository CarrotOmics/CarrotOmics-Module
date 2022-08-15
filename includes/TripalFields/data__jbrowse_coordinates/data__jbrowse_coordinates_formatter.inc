<?php

class data__jbrowse_coordinates_formatter extends ChadoFieldFormatter {

  // The default label for this field.
  public static $default_label = 'Sequence Coordinate List';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = ['data__jbrowse_coordinates'];


  /**
   *
   * @see TripalFieldFormatter::view()
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {

    $reference_term = 'data:3002';
    $fmin_term = chado_get_semweb_term('featureloc', 'fmin');
    $fmax_term = chado_get_semweb_term('featureloc', 'fmax');
    $strand_term = chado_get_semweb_term('featureloc', 'strand');
    $phase_term = chado_get_semweb_term('featureloc', 'phase');

    $locations = [];
    foreach ($items as $item) {
      if (!empty($item['value'])) {
        $srcfeature = $item['value'][$reference_term];
        $fmin = $item['value'][$fmin_term];
        $fmax = $item['value'][$fmax_term];
        $phase = $item['value'][$phase_term];
        $strand = $item['value'][$strand_term];

        // when displayed in JBrowse, add 100 b.p. margin on each side for better appearance
        $margin = 100;
        $fminmargin = $fmin>$margin?$fmin - $margin:1;
        $fmaxmargin = $fmax + $margin;
        $urlsuffix = $srcfeature . ':' . $fminmargin . '..' . $fmaxmargin;
        $highlight = $srcfeature . ':' . $fmin . '..' . $fmax;
        $display_as = $srcfeature . ':' . $fmin . '..' . $fmax . $strand;

        // determine if present in JBrowse. The feature table has had the dbxref value
        // set if present in JBrowse, see notebook page 78 July 11, 2020
        $sql = "SELECT urlprefix from {db} WHERE db_id=(
          SELECT db_id FROM {dbxref} WHERE dbxref_id=(
            SELECT dbxref_id from {feature} WHERE type_id=(
              SELECT cvterm_id FROM {cvterm} WHERE name=:chromosome AND cv_id=(
                SELECT cv_id FROM {cv} WHERE name=:sequence
              )
            ) AND uniquename=:uniquename
          )
        )";
        $results = chado_query($sql, [':chromosome' => 'chromosome',
                                      ':sequence' => 'sequence',
                                      ':uniquename' => $srcfeature]);
        if ($results) {
          $obj = $results->fetchObject();
          if ($obj) {
            // Because the urlprefix from the db table is already encoded when Tripal
            // JBrowse created it, we need to unencode it first, and add a leading '/' to
            // make the url not a relative url, and add '/data' suffix as Tripal JBrowse uses.
            $urlprefix = $obj->urlprefix;
            if (!preg_match('/^\//', $urlprefix)) {
              $urlprefix = '/' . $urlprefix;
            }
            $jbrowseurl = urldecode($urlprefix) . '/data&loc=' . $urlsuffix . '&highlight=' . $highlight;
            $display_as = t('<a href="!url">'.$display_as.'</a>', ['!url' => $jbrowseurl]);
            $locations[] = $display_as;
          }
        }

      }
    }
    if (!$locations) {
      $content = 'This feature is not available in JBrowse.';
    }
    else {
      $content = implode('<br />', $locations);
    }
    $element[0] = [
      '#type' => 'markup',
      '#markup' => $content,
    ];
  }
}
