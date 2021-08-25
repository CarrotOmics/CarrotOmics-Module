<?php
/**
 * Custom functions for the CarrotOmics site
 *
 * @param $form
 * @param $form_state
 *
 * @ingroup carrotomics_admin
 */
function carrotomics_admin_sql_form($form, &$form_state) {

  // Set the breadcrumb
  $breadcrumb = [];
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('CarrotOmics', 'admin/carrotomics/sql');
  drupal_set_breadcrumb($breadcrumb);

  // Create the form
  $form = [];

  // Add a 'Generate publication URLs' button
  $form['pub_url_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'pub_url_btn',
    '#value'  => 'Generate publication URLs',
    '#prefix' => '<div><em>This button will generate a URL for any publication lacking one,'
                 . ' if it has a DOI defined. The URL will be constructed by concatenating'
                 . ' "https://dx.doi.org/" with the DOI.<br />For example, for the DOI'
                 . ' "10.1038/ng.3565" the URL becomes "https://dx.doi.org/10.1038/ng.3565"</em></div><br />',
    '#suffix' => "<hr>",
  ];

  // Add a 'Clean MD5 Checksums' button
  $form['clean_md5_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'clean_md5_btn',
    '#value'  => 'Clean MD5 Checksums',
    '#prefix' => "<div><em>Remove MD5 checksums of null strings from the feature table, these"
                 . " have the value d41d8cd98f00b204e9800998ecf8427e</em></div><br />",
    '#suffix' => "<hr>",
  ];

  // Add a 'GBS Marker Genome Locations' button
  global $user;
  $marker_loc_filename = 'public://mcl/user/'.$user->name.'/marker_loc.tsv';
  $marker_loc_url = file_create_url($marker_loc_filename);
  $form['gbs_loc_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'gbs_loc_btn',
    '#value'  => 'GBS Marker Locations',
    '#prefix' => "<div><em>For all GBS markers that do not have a genome location,"
                 . " generate a Mainlab Chado Loader data matrix to use for the"
                 . " marker_loc sheet, generated from all GBS markers that use the"
                 . " naming scheme </em>MK012345 1_2345678</div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$marker_loc_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  // Add a 'Generate QTL Abbreviations' button
  $form['qtl_abbrev_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'qtl_abbrev_btn',
    '#value'  => 'Generate QTL Abbreviations',
    '#prefix' => "<div><em>Generate featureprop entries with MAIN::published_symbol cvterm"
                 . " for all QTL lacking them, if they are of the format </em>MK012345 anytext</em></div><br />",
    '#suffix' => "<hr>",
  ];

  // Add a 'Generate 'Update stock_featuremap Table' button
  //global $user; declared above
  $stock_featuremap_filename = 'public://mcl/user/'.$user->name.'/stock_featuremap.tsv';
  $stock_featuremap_url = file_create_url($stock_featuremap_filename);
  $form['stk_fmp_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'stk_fmp_btn',
    '#value'  => 'Update stock_featuremap Table',
    '#prefix' => "<div><em>Use publication links to populate the </em>chado.stock_featuremap<em>"
                 . " table. A publication may link to both a featuremap, as well as to one"
                 . " or more stocks, which can be a population or parents of that population."
                 . " The generated list must be manually curated before loading, because a publication"
                 . " can link to more than one map, or a map may be referenced by multiple publications,"
                 . " so extraneous items must be removed from the list."
                 . "</em></div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$stock_featuremap_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  // Add a 'Find Duplicate Publications' button
  //global $user; declared above
  $merge_pub_filename = 'public://mcl/user/'.$user->name.'/merge_pub.tsv';
  $merge_pub_url = file_create_url($merge_pub_filename);
  $form['merge_pub_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'merge_pub_btn',
    '#value'  => 'Find Duplicate Publications',
    '#prefix' => "<div><em>Loading publications from both Pubmed and National Ag Library can"
                 . " result in duplicate instances of the same publication. This button will"
                 . " generate a list of candidates that can be merged with the Mainlab Chado"
                 . " Loader merge_pub template. Candidate duplications are found with a"
                 . " combination of DOI and Title+page comparisons. The first column will"
                 . " will be the publication to be kept, the second column is the publication"
                 . " that will be deleted"
                 . "</em></div><br />",
    '#suffix' => "<div>Download the generated list from <a href=\"".$merge_pub_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  return $form;
}

/**
 * Validates the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_sql_form_validate($form, &$form_state) {
  // currently only buttons, so no validation is needed
}

/**
 * Submits the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_sql_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;

  // Gets the trigger element, i.e. which button was pressed
  $triggering_element = $form_state['triggering_element']['#name'];

  // Take action depending on which button was pressed
  if ($triggering_element == 'pub_url_btn') {
    [$nerrors, $status] = carrotomics_admin_add_doi_url();
  }
  else if ($triggering_element == 'clean_md5_btn') {
    [$nerrors, $status] = carrotomics_admin_remove_useless_checksums();
  }
  else if ($triggering_element == 'gbs_loc_btn') {
    [$nerrors, $status] = carrotomics_admin_gbs_location();
  }
  else if ($triggering_element == 'qtl_abbrev_btn') {
    [$nerrors, $status] = carrotomics_admin_qtl_abbrev();
  }
  else if ($triggering_element == 'stk_fmp_btn') {
    [$nerrors, $status] = carrotomics_admin_populate_stock_featuremap();
  }
  else if ($triggering_element == 'merge_pub_btn') {
    [$nerrors, $status] = carrotomics_admin_merge_publications();
  }
  else {
    [$nerrors, $status] = [1, "Unknown button \"$triggering_element\" was pressed"];
  }
  drupal_set_message($status, $nerrors?'error':'status');
}


/**
 * Looks up a cvterm_id from a cv and cvterm
 * Returns an array. First value is 1 if error, 0 if success.
 * Second value is error message if error, cvterm_id if success.
 *
 * @param $cv_name
 * @param $cvterm_name
 *
 */
function carrotomics_admin_lookup_cvterm_id($cv_name, $cvterm_name) {
  $sql = "SELECT cvterm_id FROM {cvterm} WHERE name = :cvterm_name AND cv_id=(SELECT cv_id FROM {cv} WHERE name = :cv_name)";
  $args = [':cv_name' => $cv_name, ':cvterm_name' => $cvterm_name];
  $typeresults = chado_query($sql, $args);
  $obj = $typeresults->fetchObject();
  if ($obj) {
    return [0, $obj->cvterm_id];
  }
  else {
    return [1, t('Error retrieving cvterm_id for "@cvterm_name" from cv "@cv_name"',
                 ['@cvterm_name' => $cvterm_name, '@cv_name' => $cv_name])];
  }
}

/**
 * For every publication that does not currently have a URL, see if it has
 * a DOI, and if so, construct a URL based on the DOI with
 *   $url = 'https://dx.doi.org/' . $doi;
 *
 */
function carrotomics_admin_add_doi_url() {
  $errors = '';

  // get cvterm_id to use for URL as type_id in pubprop table
  [$error, $type_id] = carrotomics_admin_lookup_cvterm_id('tripal_pub', 'URL');
  if ($error ){ return[$error, $type_id]; }

  // retrieve an array of all existing URL entries, key is pub_id
  $sql = "SELECT pub_id, value FROM {pubprop} WHERE type_id=$type_id";
  $args = [];
  $urlresults = chado_query($sql, $args);
  $urls = [];
  while ($obj = $urlresults->fetchObject()) {
    $urls[$obj->pub_id] = $obj->value;
  }

  // retrieve an array of all existing DOI entries, key is pub_id
  $sql = "SELECT pub_id, value FROM {pubprop} WHERE type_id=(SELECT cvterm_id FROM {cvterm} WHERE name='DOI' AND cv_id=(SELECT cv_id FROM {cv} WHERE name='tripal_pub'))";
  $args = [];
  $doiresults = chado_query($sql, $args);
  $dois = [];
  while ($obj = $doiresults->fetchObject()) {
    $dois[$obj->pub_id] = $obj->value;
  }

  // loop to add missing URLs
  $rank = 0;  // all added URLs will have this rank value. Since they are new, there is no possibility of conflict
  $nadded = 0;
  $nerrors = 0;
  foreach($dois as $pub_id => $doi) {
    // skip if this pub already has a URL
    if (!array_key_exists($pub_id, $urls)) {
      // skip any unusual DOI entries, e.g. OCLC:ocm49858565
      if ( preg_match('/^10\./', $doi) ) {
        // here we generate a new URL and add it
        $url = 'https://dx.doi.org/' . $doi;
        // equivalent SQL: INSERT INTO {pubprop} (pub_id, type_id, value, rank) VALUES ($pub_id, $type_id, $url, $rank);
        $values = [ 'pub_id' => $pub_id,
                    'type_id' => $type_id,
                    'value' => $url,
                    'rank' => $rank ];
        try {
          $result = chado_insert_record('pubprop', $values);
          if ($result) {
            $nadded++;
          }
          else {
            $nerrors++;
          }
        } catch (Exception $e) {
          $nerrors++;
          $errors .= '<br />' . $e->getMessage();
        }
      }
    }
  }

  $status = t('Added @nadded URLs, @nerrors errors', ['@nadded' => $nadded, '@nerrors' => $nerrors]);
  return [$nerrors, $status . $errors];
}

/**
 * Many features get an md5 checksum generated for a null string.
 * These are not useful. This function will remove them.
 *
 */
function carrotomics_admin_remove_useless_checksums() {
  // equivalent SQL: UPDATE chado.feature SET md5checksum = NULL WHERE md5checksum = 'd41d8cd98f00b204e9800998ecf8427e';

  $where = [ 'md5checksum' => 'd41d8cd98f00b204e9800998ecf8427e' ];
  $newvalues = [ 'md5checksum' => NULL ];

  // First get number of matching records that we will update, since
  // chado_update_record only returns true or false
  $resultarray = chado_select_record('feature', [], $where);
  $nchanged = sizeof($resultarray);

  // Now make the changes
  try {
    $success = chado_update_record('feature', $where, $newvalues);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }
  if ($success) {
    return [0, "$nchanged records updated"];
  }
  else {
    return [1, "$nchanged records to update, but something unexpected went wrong"];
  }
}

/**
 * Generate genome locations for GBS markers in the chado.featureloc table
 *
 */
function carrotomics_admin_gbs_location() {
  // It is hard to be sure which genome to use, so this function will just create
  // a table, that can then be used to create a MCL file. Populate with
  // default values for DCARv2 genome.
  $genome = 'Carrot Genome Assembly DCARv2';  // default value for the "*genome" column
  $genus = 'Daucus';
  $species = 'carota';
  $infraspecific_type = 'subspecies';
  $infraspecific_name = 'sativus';

  $headers = [ '*genome', '*chromosome', '*marker_name',
               'genus', 'species', 'infraspecific_type', 'infraspecific_name',
               'fmin', 'fmax', 'strand' ];

  $sql = "SELECT F.feature_id, F.name FROM {feature} F LEFT JOIN {featureloc} L"
       . " ON F.feature_id=L.feature_id WHERE F.name ~ '^MK\d+ \d_\d+$' AND L.featureloc_id IS NULL"
       . " AND F.type_id=(SELECT cvterm_id FROM cvterm C WHERE C.name='genetic_marker'"
       . " AND C.cv_id=(SELECT cv_id FROM cv WHERE name='sequence'))"
       . " ORDER BY F.name";
  $args = [];
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }

  // initialize the output file conent
  $content = implode("\t", $headers) . "\n";
  $nfound = 0;

  while ($obj = $results->fetchObject()) {
    $nfound++;
    $name = $obj->name;
    // extract chromosome number and position. strand is left blank
    preg_match('/(\d+)_(\d+)$/', $name, $matches);
    $chromosome = 'DCARv2_Chr' . $matches[1];
    $fmax = $matches[2];
    $fmin = $fmax - 1;
    $strand = '';
    $cols = [ $genome, $chromosome, $name,
              $genus, $species, $infraspecific_type, $infraspecific_name,
              $fmin, $fmax, $strand ];
    $content .= implode("\t", $cols) . "\n";
  }

  // save file in mcl directory. Save even if nothing retrieved, to overwrite old versions
  global $user;
  $marker_loc_filename = 'public://mcl/user/'.$user->name.'/marker_loc.tsv';
  file_put_contents($marker_loc_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound markers found, download them from the link below the button"];
  }
  else {
    return [0, "There were no markers found without genome locations"];
  }
}

/**
 * For every publication that does not currently have a URL, see if it has
 * a DOI, and if so, construct a URL based on the DOI with
 *   $url = 'https://dx.doi.org/' . $doi;
 *
 */
function carrotomics_admin_qtl_abbrev() {
  $errors = '';

  // get cvterm_id for QTL
  [$error, $qtl_type_id] = carrotomics_admin_lookup_cvterm_id('sequence', 'QTL');
  if ($error ){ return[$error, $qtl_type_id]; }

  // get cvterm_id to use for featureprop table from MAIN::published_symbol
  [$error, $symbol_type_id] = carrotomics_admin_lookup_cvterm_id('MAIN', 'published_symbol');
  if ($error ){ return[$error, $symbol_type_id]; }

  // retrieve an array of all QTL lacking the published_symbol property
  // the query retrieves all QTL featureprop, and in the loop flag those
  // that already have a published_symbol
  $sql = "SELECT DISTINCT F.feature_id, F.name, F.uniquename, FP.type_id FROM {feature} F"
       . " LEFT JOIN {featureprop} FP ON F.feature_id=FP.feature_id"
       . " WHERE F.type_id=:qtl_type_id";
  $args = [':qtl_type_id' => $qtl_type_id];
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return [1, $e->getMessage()];
  }
  $qtls = [];
  // we will often iterate the same feature_id multiple times due to multiple featureprop records
  while ($obj = $results->fetchObject()) {
    $qtls[$obj->feature_id]['name'] = $obj->uniquename;
    if ($obj->type_id == $symbol_type_id) {
      $qtls[$obj->feature_id]['exclude'] = 1;
    }
  }

  // loop to add missing published_symbol entries to the featureprop table
  $rank = 0;  // all added lines will have this rank value. Since they are new, there is no possibility of conflict
  $nadded = 0;
  $nskipped = 0;
  $nerrors = 0;
  foreach($qtls as $feature_id => $record) {
    $uniquename = $record['name'];
    $exclude = array_key_exists('exclude', $record);  // defined only when excluded
    // skip if excluded because it already has a published_symbol
    if (!$exclude) {
      // Derive name by removing the 'MK012345 ' portion. If that's missing, skip it
      if ( preg_match('/^MK\d{6} (.*)/', $uniquename, $matches) ) {
        $published_symbol = $matches[1];
        if ($published_symbol) {
          $values = [ 'feature_id' => $feature_id,
                      'type_id' => $symbol_type_id,
                      'value' => $published_symbol,
                      'rank' => $rank ];
          try {
            $result = chado_insert_record('featureprop', $values);
            if ($result) {
              $nadded++;
            }
            else {
              $nerrors++;
            }
          } catch (Exception $e) {
            $nerrors++;
            $errors .= '<br />' . $e->getMessage();
          }
        }
        else {
          $errors .= "Unparseable name \"$uniquename\"<br>";
          $nerrors++;
        }
      }
      else {
        $nskipped++;
      }
    }
  }

  $status = t('Added @nadded published_names, @nskipped skipped, @nerrors errors',
              ['@nadded' => $nadded, '@nskipped' => $nskipped, '@nerrors' => $nerrors]);
  return [$nerrors, $status . $errors];
}

/**
 * Generate a file to populate the chado.stock_featuremap table using publications to find the links.
 *
 */
function carrotomics_admin_populate_stock_featuremap() {
  // The first three columns are for the chado bulk loader. The other columns are
  // information for the manual filtering process
  $headers = ['featuremap_id', 'stock_id', 'type_id',
              '(stock type)', '(pub_id)', '(map name)', '(stock name)', '(stock uniquename)'];

  $sql = ' SELECT F.featuremap_id, F.name AS mapname, FP.pub_id, S.name AS stockname, S.uniquename, SP.stock_id, S.type_id, CV.name AS stocktype'
       . ' FROM {featuremap} F'
       . ' LEFT JOIN {featuremap_pub} FP ON F.featuremap_id=FP.featuremap_id'  // find publication(s) referencing this map
       . ' LEFT JOIN {stock_pub} SP ON FP.pub_id=SP.pub_id'  // find stocks the publication refers to
       . ' LEFT JOIN {stock} S ON SP.stock_id=S.stock_id'  // used to get type_id of stock
       . ' LEFT JOIN {cvterm} CV ON S.type_id=CV.cvterm_id'  // used to get name of type of stock
       . ' LEFT JOIN {stock_featuremap} SF ON SP.stock_id=SF.stock_id'  // used to find if already present in stock_featuremap
       . ' WHERE SP.pub_ID IS NOT NULL'  // eliminates when publication does not refer to any stock
       . ' AND SF.featuremap_id IS NULL'  // if not null it is already in stock_featuremap table, can skip
       . ' ORDER BY F.featuremap_id, SP.stock_id';
  $args = [];

  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return[1, $e->getMessage()];
  }

  // initialize the output file conent
  $content = implode("\t", $headers) . "\n";
  $nfound = 0;
  $seen = [];  // I can't easily DISTINCT with so many columns, remove duplicates in the loop

  while ($obj = $results->fetchObject()) {
    $key = $obj->featuremap_id . ':' . $obj->stock_id . ':' . $obj->type_id;
    if (!array_key_exists($key, $seen)) {
      $cols = [ $obj->featuremap_id,
                $obj->stock_id,
                $obj->type_id,
                $obj->stocktype,
                $obj->pub_id,
                $obj->mapname,
                $obj->stockname,
                $obj->uniquename,
              ];
      $content .= implode("\t", $cols) . "\n";
      $nfound++;
      $seen[$key] = 1;
    }
  }

  // save file in mcl directory. Save even if nothing retrieved, to overwrite old versions
  global $user;
  $marker_loc_filename = 'public://mcl/user/'.$user->name.'/stock_featuremap.tsv';
  file_put_contents($marker_loc_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound possible map to stock references found, download them from the link below the button"];
  }
  else {
    return [0, "There were no map to stock references found"];
  }
}

/**
 * Generate a file to merge duplicate publications
 *
 */
function carrotomics_admin_merge_publications() {
  // The first three columns are for the Mainlab Chado Loader. The other columns are
  // information for the manual filtering process
  $headers = ['*pub_id_base', '*pub_id', 'delete',
              '(property)', '(duplicated value)',
              '(year_base)', '(year)',
              '(volume_base)', '(volume)',
              '(issue_base)', '(issue)',
              '(page_base)', '(page)',
              '(journal_base)', '(journal)',
              '(title_base)', '(title)'];
  // characters to remove to make a title hash key
  // sometimes one version of a title ends in a period, other times it doesn't
  $titlehashpattern = ['/\.+/', '/\s+/'];

  // this will store key=>lower pub_id value=>[higher pub_id, info]
  $candidates = [];
  $pub_info = [];  // cache of other information from each pub, key is pub_id

  // Duplicate detection part 1 = based on specific properties
  // these cvterms will be used to find duplicates, based on just that single value
  $cvtermnames = ['DOI', 'Elocation', 'URL'];
  foreach ($cvtermnames as $cvtermname) {
    $sql = "SELECT A.pub_id, A.value, A.rank,"
         . " P.title, P.series_name, P.pyear, P.volume, P.issue, P.pages"
         . " FROM {pubprop} A"
         . " JOIN (SELECT value, count(*) FROM {pubprop} PP"
         . " WHERE PP.type_id="
         . "  (SELECT cvterm_id FROM {cvterm} CVT WHERE cv_id=(SELECT cv_id FROM {cv} CV"
         . "   WHERE CV.name=:cvname) AND CVT.name=:cvtermname)"
         . " GROUP BY value HAVING COUNT(*) > 1) B ON A.value=B.value"
         . " LEFT JOIN {pub} P ON A.pub_id=P.pub_id"
         . " ORDER BY A.value, A.pub_id";
    $args = [':cvname' => 'tripal_pub', ':cvtermname' => $cvtermname];

    try {
      $results = chado_query($sql, $args);
    } catch (Exception $e) {
      return[1, $e->getMessage()];
    }

    // Based on the SQL there will be at least two duplicates of a given
    // value, but there may be more. For each set of duplicates, we will
    // store to the candidates hash, with the lowest pub_id as the key.
    // The SQL is sorted, so the first one will always be the lowest pub_id.
    $prevvalue = '';
    $first_pub_id = '';
    while ($obj = $results->fetchObject()) {
      $pub_id = $obj->pub_id;
      $value = $obj->value;

      if ($value != $prevvalue) {
        // first of a new set of duplicates, just save the pub_id
        $first_pub_id = $pub_id;
        $prevvalue = $value;
      }
      else {
        // second or later of a set of duplicates
        $candidates[$first_pub_id][] = [$pub_id, $cvtermname, $obj->value];
      }
    }
  }  // foreach $cvtermnames

  // Duplicate detection part 2 = based on title
  // Try to find duplicates based just on nearly identical titles combined
  // with starting page number
  $sql = "SELECT P.pub_id, P.title, P.series_name, P.pyear, P.volume, P.issue, P.pages"
       . " FROM {pub} P"
       . " ORDER BY P.pub_id";
  $args = [];
  try {
    $results = chado_query($sql, $args);
  } catch (Exception $e) {
    return[1, $e->getMessage()];
  }

  // load every publication and store by processed title + page
  $titlehash = [];
  while ($obj = $results->fetchObject()) {
    $pub_id = $obj->pub_id;

    // cache other information from pub for output file
    $pub_info[$pub_id]['year'] = $obj->pyear;
    $pub_info[$pub_id]['volume'] = $obj->volume;
    $pub_info[$pub_id]['issue'] = $obj->issue;
    $pub_info[$pub_id]['firstpage'] = preg_replace('/\-.*/', '', $obj->pages);
    $pub_info[$pub_id]['journal'] = $obj->series_name;
    $pub_info[$pub_id]['title'] = $obj->title;

    $title = preg_replace($titlehashpattern, [], strtolower($obj->title));
    $page = preg_replace('/\-.*/', '', $obj->pages);
    if ($title) {
      $key = $title . '.' . $page;
      $titlehash[$key][] = $pub_id;
    }
  }

  // now look for titlehash keys with more than one value
  foreach ($titlehash as $key => $arrayref) {
    if ( count($arrayref) > 1 ) {
      $pub_ids = $arrayref;
      sort($pub_ids);  // get lowest pub_id as first element
      for ($i=1; $i<count($pub_ids); $i++) {
        $candidates[$pub_ids[0]][] = [$pub_ids[$i], 'matching title', ''];
      }
    }
  }

  // initialize the output file conent
  $content = implode("\t", $headers) . "\n";
  $nfound = 0;

  // populate output file
  ksort($candidates);
  foreach ($candidates as $pub_id => $arrayref) {
    foreach ($arrayref as $idandvalue) {
      $dup_pub_id = $idandvalue[0];
      $dup_pub_cvterm = $idandvalue[1];
      $dup_pub_value = $idandvalue[2];
      $cols = [ $pub_id, $dup_pub_id, 'yes',
                $dup_pub_cvterm, $dup_pub_value ];
      // additional informational columns
      $coltypes = ['year', 'volume', 'issue', 'firstpage', 'journal', 'title'];
      $flip_base_votes = 0;
      foreach ($coltypes as $coltype) {
        $left = $pub_info[$pub_id][$coltype];
        $right = $pub_info[$dup_pub_id][$coltype];
        if ($coltype == 'title') {
          // for title, case insensitive, and ignore white space and periods
          if (preg_replace($titlehashpattern, [], strtolower($left)) == preg_replace($titlehashpattern, [], strtolower($right))) {
            $right = '=';
          }
        }
        else {
          // If one publication has volume, page, etc and the other doesn't,
          // then that one should become the base_id. Vote on it
          if (($left) and (!$right)) {
            $flip_base_votes--;  // negative = don't flip
          }
          elseif ((!$left) and ($right)) {
            $flip_base_votes++;  // positive = do flip
          }
          // if equal, abbreviate the second copy
          elseif (($left) and ($right) and ($left == $right)) {
            $right = '=';
          }
        }
        $cols[] = $left;
        $cols[] = $right;
      }
      // check votes, and if positive swap (flip) pub_id_base and pub_id
      if ($flip_base_votes > 0) {
        $tmp = $cols[0];
        $cols[0] = $cols[1];
        $cols[1] = $tmp;
      }
      $content .= implode("\t", $cols) . "\n";
      $nfound++;
    }
  }

  // save file in mcl directory. Save even if nothing retrieved, to overwrite old versions
  global $user;
  $merge_pub_filename = 'public://mcl/user/'.$user->name.'/merge_pub.tsv';
  file_put_contents($merge_pub_filename, $content);

  if ( $nfound ) {
    return [0, "$nfound publications may need to be merged, download them from the link below the button"];
  }
  else {
    return [0, "There were no publications found that are candidates for merging"];
  }
}
