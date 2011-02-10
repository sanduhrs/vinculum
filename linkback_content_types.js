// $Id$
(function ($) {
Drupal.behaviors.contentTypes = {
  attach: function (context) {
    // Provide the vertical tab summaries.
    $('fieldset#edit-linkback', context).drupalSetSummary(function(context) {
      // The summary is one of:
      // - Send and receive
      // - Send only
      // - Receive only
      // - Disabled
      send = $("input[name='linkback_send']", context).is(':checked');
      receive = $("input[name='linkback_receive']", context).is(':checked');

      if (send && receive) {
        summary = Drupal.t('Send and receive');
      }
      else if (send || receive) {
        summary = send ? Drupal.t('Send only') : Drupal.t('Receive only');
      }
      else {
        summary = Drupal.t('Disabled');
      }
      return summary;
    });
  }
};
})(jQuery);
