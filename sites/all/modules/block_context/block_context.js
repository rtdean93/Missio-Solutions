jQuery(document).ready(function() { 
	
  jQuery('#block-context-select').change( function() {
	
    window.location.href = jQuery(this).val();
  });
});