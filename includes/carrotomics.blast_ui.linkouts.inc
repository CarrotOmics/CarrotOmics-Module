<?php

/**
 * @file
 * Provides Link-out functionality for BLAST hits.
 *
 * Specifically, this is how the URL portion of the hit links in your users
 * Blast results is formed.
 *
 * To implement your own link-out type:
 *  1) Register your link-out type by implementing hook_blast_linkout_info().
 *      Hook blast_linkout_info must return an array of types where each type
 *      has a name and process function. For example,
 * @code
  function blast_ui_blast_linkout_info() {
    $types = array();

    $types['my-link-type'] = array(
      'name' => 'My Amazing Link Type',
      'process function' => 'mymodule_generate_linkout_mylinktype',
    );

    return $types;
  }
 * @endcode
 *
 *  2) Implement the process function you specified to determine the URL
 *     to be linked to depending on the blast hit. For a full description of
 *     parameters available to your function, see tripal_blast_generate_linkout_link().
 * @code
  function mymodule_generate_linkout_mylinktype($url_prefix, $hit, $info, $options = array()) {

    // Do some simple steps to generate the suffix based on the $hit.

    return l('url name', $url_prefix . $url_postfix);
  }
 * @endcode
 *
 * This module will automatically,
 *  - Add your custom type to the "Link-out Type" select list on Blast Database
 *    node add/edit forms.
 *  - If your type is chosen by the user when the Blast Database is created,
 *    then your process function will be used by blast_report.tpl.php to
 *    determine the URL that should be used for each hit link.
 */

/**
 * Implements hook_blast_linkout_info().
 * Provide information on basic link-out types: link, GBrowse, JBrowse.
 *
 * NOTE: Each item must have a 'name' and 'process function' to indicate the
 * human-readable name to be used in the Blast Database add/edit form and the
 * function to be used to determine the URL for each hit in BLAST results.
 */

function carrotomics_blast_linkout_info() {
  $types = [];

  $types['carrotomics_jbrowse'] = array(
    'name' => 'CarrotOmics JBrowse',
    'process function' => 'carrotomics_generate_linkout_carrotomicsjbrowse',
    'help' => 'The link created will add a "Blast Result" track to the JBrowse (specified by the 
      External Database) that shows the HSPs as well as indicating the overall hit. 
      <strong><em>It is assumed that the Reference of the JBrowse is the same as this BLAST 
      database (even the names must be consistent).</em></strong> Furthermore, the URL prefix 
      supplied is expected to have an empty query (?) or be properly ended (&). For
      example, "http://mydomain.com/jbrowse/tripalus_databasica/?" OR
      "http://mydomain.com/jbrowse/tripalus_databasica/?tracks=genes,markers,blast&". 
      Also <strong><em>the Blast Result track is NOT Displayed by default</em></strong>. Either include "blast" 
      using the "tracks" directive in the URL prefix or specify it in your JBrowse.conf.',
    // Whether or not the link-out requires additional fields from the nodes.
    'require_regex' => TRUE,
    'require_db' => TRUE,
  );

  return $types;
}

/**
 * Generate a JBrowse link-out with location information for a given hit.
 *
 * NOTE: Assumes the hit is a backbone feature in the JBrowse linked to.
 *  Otherwise, the basic link can be used.
 * NOTE: This linkout creates a "blast" track but doesn't make it visible. This is to
 *  allow your default tracks to be visible and give contect to your blast hit. You
 *  should include "blast" in your jbrowse.conf default track list to ensure your
 *  users can always see their hits. If you don't have access to the jbrowse.conf,
 *  you can place the tracks you want to see including 'blast' in the url prefix
 *  (see example below under @param $url_prefix).
 *
 * @param $url_prefix
 *   The URL prefix for the BLAST Database queried. It is assumed that the url prefix
 *   includes the ? and if there are any key=vale pairs that the last symbol is &.
 *   For example,
 *     http://myserver.com/jbrowse/databasica/?
 *     http://myserver.com/jbrowse/databasica/?tracks=myfavtrack,anoktrack,blast&
 *
 * @param $hit
 *   The blast XML hit object. This object has the following keys based on the
 *   XML: Hit_num, Hit_id, Hit_def, Hit_accession, Hit_len and Hit_hsps.
 *   Furthermore, the following keys have been added:
 *    -linkout_id: the part of the Hit_def extracted using a regex provided
 *     when the blastdb node was created.
 *    -hit_name: the name of the hit extracted in the template.
 * @param $info
 *   Additional information that may be useful in creating a link-out. Includes:
 *    - query_name: the name of the query sequence.
 *    - score: the score of the blast hit.
 *    - e-value: the e-value of the blast hit.
 * @param $options
 *   Any additional options needed to determine the type of link-out. None are
 *   supported by this particular link-out type.
 *
 * @return
 *   An html link.
 */
function carrotomics_generate_linkout_carrotomicsjbrowse($url_prefix, $hit, $info, $options = array()) {

  // First we need to collect the HSPs to define the ranges we want to
  // display on the JBrowse.
  $ranges = array();
  // We also keep track of all the coordinates in order to later
  // calculate the smallest and largest coordinate.
  $coords = array();
  $count = 0;
  $strands = array();
  foreach($info['HSPs'] as $hsp) {
    $count++;

    $strand = '1';
    $hsp_start = $hsp['Hsp_hit-from'];
    $hsp_end = $hsp['Hsp_hit-to'];
    $strands[] = $strand;

    // Handle alignments on the negative strand.
    if (($hsp_end - $hsp_start) < 0) {
      $strand = '-1';
      $hsp_start = $hsp['Hsp_hit-to'];
      $hsp_end = $hsp['Hsp_hit-from'];
    }

    // Add both the start & stop to the coordinate list.
    array_push($coords,$hsp['Hsp_hit-from'] , $hsp['Hsp_hit-to'] );

    // Format the hsp for inclusion in the subfeatures section of the track later.
    $hsp_def = format_string(
      '{"start":!start,"end":!end,"strand":"!strand","type":"!type"}',
      array(
        '!start' => $hsp_start,
        '!end' => $hsp_end,
        '!strand' => $strand,
        '!type' => 'match_part'
      )
    );
    array_push($ranges, $hsp_def);
  }
  // Calculate the minimum & maximum coordinates.
  $min = min($coords);
  $max = max($coords);

  // We also want some white-space on either side of out hit
  // when we show it in the JBrowse. To make this generic,
  // we want our blast hit to take up 2/3 of the screen thus
  // we have 1/6 per side for white-space.
  $buffer = round(($max - $min) / 6);
  $screen_start = $min - $buffer;
  $screen_end = $max + $buffer;

  // Now we are finally ready to build the URL.
  // First lets set the location of the hit so the JBrowse focuses in on the correct region.
  $jbrowse_query = array();
  $jbrowse_query['loc'] = format_string(
    'loc=!ref:!start..!stop',
    array(
      '!ref' => $hit->{'linkout_id'},
      '!start' => $screen_start,
      '!stop' => $screen_end,
    )
  );

  $unique_strands = array_unique($strands);
  if (count($unique_strands) === 1) {
    $strand = end($strands);
    // Next we want to add our BLAST hit to the JBrowse.
    $jbrowse_query['addFeatures'] = format_string(
      'addFeatures=[{"seq_id":"!id","start":!min,"end":!max,"name":"!name","strand":!strand,"subfeatures":[!hspcoords]}]',
      array(
        '!id' => $hit->{'linkout_id'},
        '!name' => $info['query_name'] . ' Blast Hit',
        '!min' => $min,
        '!max' => $max,
        '!hspcoords' => join(",", $ranges),
        '!strand' => $strand,
      )
    );
  }
  else {
    $jbrowse_query['addFeatures'] = format_string(
      'addFeatures=[{"seq_id":"!id","start":!min,"end":!max,"name":"!name","subfeatures":[!hspcoords]}]',
      array(
        '!id' => $hit->{'linkout_id'},
        '!name' => $info['query_name'] . ' Blast Hit',
        '!min' => $min,
        '!max' => $max,
        '!hspcoords' => join(",", $ranges),
      )
    );
  }

  // Then add a track to display our new feature.
  $jbrowse_query['addTracks'] = 'addTracks=[{"label":"blast","key":"BLAST Result","type":"JBrowse/View/Track/CanvasFeatures","store":"url"}]';

  $url_postfix = implode('&', $jbrowse_query);

  $hit_url = $url_prefix . $url_postfix;

  $tmp = l(
    $hit->{'linkout_id'},
    $hit_url,
    array('attributes' => array('target' => '_blank'))
  );

  // don't know why, but these cannot be escaped
  $tmp = preg_replace('/%3F/', '?', $tmp);
  $tmp = preg_replace('/%26/', '&', $tmp);
  $tmp = preg_replace('/%3D/', '=', $tmp);

  return $tmp;
}
