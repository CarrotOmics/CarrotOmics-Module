<?php
/**
 * Custom functions for the CarrotOmics site
 *
 * @param $form
 * @param $form_state
 *
 * @ingroup carrotomics_admin
 */
function carrotomics_admin_pub_form($form, &$form_state) {

  // Set the breadcrumb
  $breadcrumb = [];
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('CarrotOmics', 'admin/carrotomics/pub');
  drupal_set_breadcrumb($breadcrumb);

  global $user;  // used for output file location

  // Create the form
  $form = [];

  // Add a 'Generate publication URLs' button
  $form['pub_url_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'pub_url_btn',
    '#value'  => 'Generate publication URLs',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . 'This button will generate a URL for any publication lacking one,'
                 . ' if it has a DOI defined. The URL will be constructed by concatenating'
                 . ' "https://dx.doi.org/" with the DOI.<br />For example, for the DOI'
                 . ' "10.1038/ng.3565" the URL becomes "https://dx.doi.org/10.1038/ng.3565"'
                 . '</em></div><br />',
    '#suffix' => "<hr>",
  ];

  // Add a 'Get maximum pub_id' button
  $form['pub_max_pub_id_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'max_pub_id_btn',
    '#value'  => 'Get maximum pub_id value',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "This button returns the highest value of pub_id in the chado.pub table."
                 . " This can be used in conjunction with the Find Duplicate Publications"
                 . " button below if the value is retrieved prior to loading additional publications."
                 . "</em></div><br />",
    '#suffix' => "<hr>",
  ];

  // Add a 'Find Duplicate Publications' button and optional minimum value for pub_id
  $merge_pub_filename = 'public://mcl/user/'.$user->name.'/merge_pub.tsv';
  $merge_pub_url = file_create_url($merge_pub_filename);
  $form['merge_pub_min_pub_id'] = [
    '#type'     => 'textfield',
    '#name'     => 'merge_pub_min_pub_id',
    '#title'    => 'Optional minimum pub_id value',
    '#required' => FALSE,
    '#prefix'   => '<div style="padding-top:30px;"><em>'
                 . 'Loading publications from both Pubmed and National Ag Library can'
                 . ' result in duplicate instances of the same publication. This button will'
                 . ' generate a list of candidates that can be merged with the Mainlab Chado'
                 . ' Loader </em>merge_pub<em> template. Candidate duplications are found with a'
                 . ' combination of DOI and Title comparisons. The first column will'
                 . ' be the publication to be kept, the second column is the publication'
                 . ' that will be deleted. Several additional columns are included for'
                 . ' manual examination.'
                 . '</em></div><br />',
  ];
  $form['merge_pub_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'merge_pub_btn',
    '#value'  => 'Find Duplicate Publications',
    '#suffix' => "<div>Download the generated list from <a href=\"".$merge_pub_url."\">here</a></div><br />"
                 . "<hr>",
  ];

  // Add a 'Find Unlinked Errata' button
  $form['unlinked_pub_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'unlinked_pub_btn',
    '#value'  => 'Find Unlinked Errata',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . 'Find publications that are erratum or correction publications'
                 . ' that have not yet been linked to the original publication'
                 . ' as a relationship ("tripal_pub" CV "part of" cvterm) and create'
                 . ' a link between them in the chado.pub_relationship table'
                 . '</em></div><br />',
    '#suffix' => "<hr>",
  ];

  // Add a 'Update publication Unique Local Identifier' button
  $form['pub_uli_btn'] = [
    '#type'   => 'submit',
    '#name'   => 'pub_uli_btn',
    '#value'  => 'Update publication Unique Local Identifier',
    '#prefix' => '<div style="padding-top:30px;"><em>'
                 . "The \"uniquename\" column in the chado.pub table is named"
                 . " \"Unique Local Identifier\" in the widget. These values are not displayed"
                 . " to the end user. The publication loaders generate this to be the publication"
                 . " citation, which is long, and it is not used on CarrotOmics."
                 . " Replace these with the pub_id value."
                 . "</em></div><br />",
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
function carrotomics_admin_pub_form_validate($form, &$form_state) {
  // currently only buttons, so no validation is needed
}

/**
 * Submits the form.
 *
 * @param $form
 * @param $form_state
 */
function carrotomics_admin_pub_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;

  // Gets the trigger element, i.e. which button was pressed
  $triggering_element = $form_state['triggering_element']['#name'];

  // Gets optional values
  $min_pub_id = $form_state['values']['merge_pub_min_pub_id'];

  // Take action depending on which button was pressed
  if ($triggering_element == 'pub_url_btn') {
    [$nerrors, $status] = carrotomics_admin_add_doi_url();
  }
  else if ($triggering_element == 'max_pub_id_btn') {
    [$nerrors, $status] = carrotomics_admin_max_pub_id();
  }
  else if ($triggering_element == 'merge_pub_btn') {
    [$nerrors, $status] = carrotomics_admin_merge_publications($min_pub_id);
  }
  else if ($triggering_element == 'unlinked_pub_btn') {
    [$nerrors, $status] = carrotomics_admin_unlinked_pub();
  }
  else if ($triggering_element == 'pub_uli_btn') {
    [$nerrors, $status] = carrotomics_admin_pub_uli();
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
function carrotomics_admin_lookup_pub_cvterm_id($cv_name, $cvterm_name) {
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
  [$error, $type_id] = carrotomics_admin_lookup_pub_cvterm_id('tripal_pub', 'URL');
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
 * Find errata and corrections that should be linked to the original publication
 *
 */
function carrotomics_admin_max_pub_id() {
  $outputmessage = '';

  // Simple SQL to return the highest value of pub_id in the chado.pub table
  $sql = "SELECT MAX(pub_id) AS max_pub_id FROM {pub}";
  $args = [];
  $results = chado_query($sql, $args);

  // Loop and investigate each retrieved publication
  while ($obj = $results->fetchObject()) {
    $max_pub_id = $obj->max_pub_id;
  }

  if ($max_pub_id) {
    return [0, "Maximum pub_id = $max_pub_id"];
  }
  else {
    return [1, "Error from \"$sql\""];
  }
}

/**
 * Generate a file to merge duplicate publications
 *
 */
function carrotomics_admin_merge_publications($min_pub_id = NULL) {
  // The first three columns are for the Mainlab Chado Loader. The other columns are
  // information for the manual filtering process
  $headers = ['*pub_id_base', '*pub_id', 'delete',
              '(entity_base)', '(entity)',
              '(property)', '(duplicated value)',
              '(year_base)', '(year)',
              '(volume_base)', '(volume)',
              '(issue_base)', '(issue)',
              '(page_base)', '(page)',
              '(journal_base)', '(journal)',
              '(title_base)', '(title)'];
  // Characters to remove to make a title hash key.
  // Sometimes one version of a title ends in a period, other times it doesn't.
  // Dashes and ndashes may be subsituted for each other.
  // Remove any HTML tags.
  // Quotes single or double can vary, remove them.
  $titlehashpattern = ['/\.+/', '/\s+/', '/\-+/', '/‐+/', '/—+/', '/<[^>]+>/',
                       '/‘/', '/’/', "/'/", '/“/', '/”/', '/"/'];

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
      // consider http:// and https:// equivalent for our comparisons
      if ($cvtermname == 'URL') {
        $value = preg_replace('|^http://|i', 'https://', $value);
      }

      if ($value != $prevvalue) {
        // first of a new set of duplicates, just save the pub_id
        $first_pub_id = $pub_id;
        $prevvalue = $value;
      }
      else {
        // second or later of a set of duplicates
        $candidates[$first_pub_id][] = [$pub_id, $cvtermname, $value];
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

  // load every publication and store by processed title
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
      $key = $title;  // no longer include page here, some duplicates are missing page number
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
      $entity = chado_get_record_entity_by_table('pub', $pub_id);
      $dup_entity = chado_get_record_entity_by_table('pub', $dup_pub_id);
      $cols = [ $pub_id, $dup_pub_id, 'yes',
                $entity, $dup_entity,
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
      // filtering for optional minimum for pub_id
      if ((!$min_pub_id) or ($pub_id >= $min_pub_id) or ($dup_pub_id >= $min_pub_id)) {
        // Ignore if match key is URL and it is a generic URL
        if (($dup_pub_cvterm != 'URL') or (carrotomics_admin_valid_url($dup_pub_value))) {
          $content .= implode("\t", $cols) . "\n";
          $nfound++;
        }
      }
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

/**
 * Determine if the URL for a publication is a "general" journal URL rather than
 * directly to the publication. Returns true if the URL is valid to consider
 *
 */
function carrotomics_admin_valid_url($url) {
  $result = 1;
  // ignore if URL is just a site and no further path e.g. "http://www.jhortscib.com/"
  if (preg_match('|https?://[^/]+/$|i', $url)) {
    $result = 0;
  }
  // ignore if URL ends in "/contents" e.g. "http://www.kluweronline.com/issn/0032-079X/contents"
  elseif (preg_match('|/contents$|i', $url)) {
    $result = 0;
  }
  return $result;
}

/**
 * Find errata and corrections that should be linked to the original publication
 *
 */
function carrotomics_admin_unlinked_pub() {
  $outputmessage = '';
  $nadded = 0;
  $nexisting = 0;
  $nwarnings = 0;
  $nerrors = 0;

  // Determine the cvterm_id for tripal_pub::part of
  // It is potentially possible to also use relationship::part_of, but so
  // far CarrotOmics isn't using that term for this purpose
  [$error, $part_of_id] = carrotomics_admin_lookup_pub_cvterm_id('tripal_pub', 'part of');
  if ($error ){ return[$error, $part_of_id]; }

  // Patterns to be removed from the title to derive the original title  
  // Correction: Blah blah carrot
  // Publisher Correction: Blah blah roots
  // Correction to "Blah blah orange"
  // Erratum: Blah blah apiaceae
  // Erratum to: Blah blah daucus
  // Erratum.
  // Blah blah consumption [Erratum: 2007 Nov., v. 46, issue 2, p. 199.]
  $titlehashpattern = ['/^erratum to: */i', '/^erratum[:\.] */i',
                       '/^correction to: */i', '/^correction[:\.] */i', '/^publisher correction[:\.] */i',
                       '/^corrigendum[:\.] */i',
                       '/ *\[erratum[^\]]+\] */i'];

  // main query for candidate erratum publications
  $sql1 = "SELECT pub_id, title FROM {pub}"
        . " WHERE lower(title) LIKE '%erratum%'"
        . " OR lower(title) LIKE '%correction%'"
        . " OR lower(title) LIKE '%corrigendum%'";
  $args1 = [];
  $results1 = chado_query($sql1, $args1);

  // Loop and investigate each retrieved publication
  while ($obj1 = $results1->fetchObject()) {
    $pub_id = $obj1->pub_id;
    $title = $obj1->title;
    $title_lc = strtolower($title);
    $entity = chado_get_record_entity_by_table('pub', $pub_id);

    // Before proceeding, see if a relationship already exists. This may have been
    // done manually for some of the publications with errors or misspellings in
    // the title or other anomalies, or was done in a previous call to this function
    $sql2 = "SELECT pub_relationship_id, object_id FROM {pub_relationship}"
          . " WHERE subject_id=:subject_id AND type_id=:type_id";
    $args2 = [':subject_id' => $pub_id, ':type_id' => $part_of_id];
    $results2 = chado_query($sql2, $args2);
    $hits = [];
    while ($obj2 = $results2->fetchObject()) {
      $hits[] = $obj2->object_id;
    }
    if (sizeof($hits) > 0) {
      $nexisting++;
    }
    else {
      // Here we try to extract the original title
      $origtitle = preg_replace($titlehashpattern, '', $title_lc);
      // special handling for quoted titles e.g. https://carrotomics.org/bio_data/64646
      $origtitle = preg_replace(['/^correction to "(.*?)"(\.?)$/'], '$1$2', $origtitle);
      $origtitle = preg_replace(['/^corrigendum to "(.*?)"(\.?)$/'], '$1$2', $origtitle);

      // If this processing did nothing, it might be due to the word "correction"
      // elsewhere the title. These cases we return as warnings.
      if ($title_lc == $origtitle) {
        $nwarnings++;
        $outputmessage .= "<li>Warning: Could not extract an original title from \""
                       . l("$title", "bio_data/$entity")
                       . "\" - It may not be an actual erratum or correction</li>";
      }
      else {
        // try to retrieve the pub_id of the original publication, this is
        // a case-insensitive query
        $sql3 = "SELECT pub_id FROM {pub} WHERE lower(title) = :origtitle";
        $args3 = [':origtitle' => $origtitle];
        $results3 = chado_query($sql3, $args3);

        // we want exactly one match. Zero means we couldn't find it, two
        // or more if we have ambiguous matches
        $orig_pub_id = [];
        while ($obj3 = $results3->fetchObject()) {
          $orig_pub_id[] = $obj3->pub_id;
        }
        $nmatches = sizeof($orig_pub_id);
        if ($nmatches == 0) {
          $nerrors++;
          $outputmessage .= "<li>Error: No match to extracted original title from \""
                       . l("$title", "bio_data/$entity")
                       . "\" = \"$origtitle\""
                       . " - Perhaps the original publication needs to be added first</li>";
        }
        elseif ($nmatches > 1) {
          $nerrors++;
          $outputmessage .= "<li>Error: $nmatches ambiguous matches to extracted original title \"$origtitle\" from \""
                       . l("$title", "bio_data/$entity")
                       . "\"</li>";
        }
        else {
          // Here we now have one erratum pub and one original pub to link it to.
          // We know that a relationship does not already exist, because of the
          // query in sql2, so we can proceed to add the relationship now.
          $values = [ 'subject_id' => $pub_id,
                      'object_id' => $orig_pub_id[0],
                      'type_id' => $part_of_id ];
          try {
            $parent_entity = chado_get_record_entity_by_table('pub', $orig_pub_id[0]);
            $result = chado_insert_record('pub_relationship', $values);
            if ($result) {
              $nadded++;
              $outputmessage .= "<li>Linked publication \""
                              . l("$title", "bio_data/$entity")
                              . "\" to be part of \""
                              . l("$origtitle", "bio_data/$parent_entity")
                              . "\"</li>";
            }
            else {
              $nerrors++;
              $outputmessage .= "<li>Failure linking \"$title\" to \"$origtitle\"</li>";
            }
          } catch (Exception $e) {
            $nerrors++;
            $outputmessage .= '<li>' . $e->getMessage() . " title=\"$title\"</li>";
          }
        }
      }
    }
  }

  if ($outputmessage) {
    return [$nerrors, "$nexisting existing relationships present, $nadded relationships added,"
                      ." $nerrors errors, $nwarnings warnings<ol>$outputmessage</ol>"];
  }
  else {
    return [0, "No unlinked errata found"];
  }
}

/**
 * Change the Unique Local Identifier to be pub_id
 *
 */
function carrotomics_admin_pub_uli() {
  $errors = '';

  // Find the publications needing to be updated
  $sql = "SELECT pub_id, uniquename FROM {pub} P"
       . " WHERE NOT pub_id::text=uniquename";
  $args = [];
  $results = chado_query($sql, $args);

  // Loop and make changes, keeping track of the number of changes made
  $nchanged = 0;
  $nerrors = 0;
  while ($obj = $results->fetchObject()) {
    $pub_id = $obj->pub_id;
    $uniquename = $obj->uniquename;
    $where = [ 'pub_id' => $pub_id ];
    $newvalues = [ 'uniquename' => $pub_id ];
    try {
      $success = chado_update_record('pub', $where, $newvalues);
      if ($success) { 
        $nchanged++;
      }
      else {
        $nerrors++;
      }
    } catch (Exception $e) {
      $nerrors++;
      $errors = '<br>' . $e->getMessage();
    }
  }

  if ($nchanged or $nerrors) {
    $status = t('Changed @nchanged uniquename values, @nerrors errors', ['@nchanged' => $nchanged, '@nerrors' => $nerrors]);
  }
  else {
    $status = t('No changes were needed');
  }
  return [$nerrors, $status . $errors];
}
