<?php

/**
 * @file
 * API functions provided by the CarrotOmics module.
 *
 * Provides an application programming interface (API) for custom functions
 * in the CarrotOmics module.
 *
 */



/**
 * Looks up a cvterm_id from the name of a cv and name of a cvterm.
 * Returns an array. First value is 1 if error, 0 if success.
 * Second value is error message if error, cvterm_id if success.
 *
 * @param $cv_name
 * @param $cvterm_name
 *
 */
function carrotomics_lookup_cvterm_id($cv_name, $cvterm_name) {
  $sql = "SELECT cvterm_id FROM {cvterm} WHERE name = :cvterm_name" 
       . " AND cv_id=(SELECT cv_id FROM {cv} WHERE name = :cv_name)";
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

