/*  Copyright 2009 David Gilbert.
    This file is jquery.jqTOC.js; you can redistribute it and/or modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

    For documentation refer to: http://solidgone.org/Jqtoc
*/
(function($){
$.fn.jqTOC = function(settings) {

   function tocToggleDisplay(e){
		$('#'+settings.tocContainer+' .toc_content')[e.data.mode]();
   }
   settings = $.extend({
      tocWidth: '220px',
      tocTitle: 'Content',
      tocStart: 1,
      tocEnd  : 3,
      tocContainer : 'toc_container',
      tocAutoClose : true,
      tocShowOnClick : true,
      tocTopLink   : 'top'
   }, settings || {});

   // create the main content container if not already created
   if (document.getElementById(settings.tocContainer) == null)
      $('body').append('<div id="'+settings.tocContainer+'"></div>');

   $('#'+settings.tocContainer).css('width',settings.tocWidth).append(
      (settings.tocTitle?'<div class="toc_header">'+ settings.tocTitle + '</div>':'') +
      '<div class="toc_content"></div>'
   );

   var t = $('#'+settings.tocContainer+' .toc_content');
   var headerLevel,headerId;

// Find the highest level heading used within the range tocStart..tocEnd. Prevents indenting when no higher level exists.
   var start=settings.tocEnd;
   this.children().each(function(i) {
      headerLevel = this.nodeName.substr(1);
      if(
      	this.nodeName.match(/^H\d+$/)
      	&& headerLevel >= settings.tocStart
      	&& headerLevel <= settings.tocEnd
      	&& this.nodeName.substr(1) < start
      ) {
      	start = this.nodeName.substr(1);
     	}
		if (start == settings.tocStart) {
		   return false;
		}
	});
	settings.tocStart=start;

   this.children().each(function(i) {
      headerLevel = this.nodeName.substr(1);
      if(
         this.nodeName.match(/^H\d+$/)
         && headerLevel >= settings.tocStart
         && headerLevel <= settings.tocEnd
      ) {
         headerId = this.id || 'jqTOC_link' + i;
         t.append('<a href="#'+ headerId +'" style="margin-left: ' + (headerLevel-settings.tocStart+1)*1.4 +'em;" ' +
            (headerLevel != settings.tocStart ? 'class="indent" ': '') +
            '>'+ $(this).text() +'</a>'
         );
         this.id = headerId;
         if (settings.tocTopLink) {
            $(this).before('<div class="toc_top"><a href="#">'+settings.tocTopLink+'</a></div>');
         }
      }
   });

	if (settings.tocShowOnClick) {
	   if (settings.tocTitle) {
	      $('#'+settings.tocContainer+' .toc_header').bind('click', {mode: 'toggle'}, function(e){tocToggleDisplay(e);});
	   }
	   if (settings.tocAutoClose) {
         $('#'+settings.tocContainer+' .toc_content a').bind('click', {mode: 'hide'}, function(e){tocToggleDisplay(e);});
      }
   } else {
      $('#'+settings.tocContainer+' .toc_content').show();
   }
   if (settings.tocTopLink) {
      $('.toc_top').bind('click', function(){window.scrollTo(0,0);});
   }
   return this;
}
})(jQuery);