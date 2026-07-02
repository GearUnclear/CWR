/* ============================================================
   USLU DUR! :: retro.js
   Period-authentic client-side trickery, no frameworks.
   Pure DOM. Degrades gracefully with JS off.
   ============================================================ */

/* ---- 1. Fake hit counter (odometer style, persisted in localStorage) ---- */
function initCounter() {
	var el = document.getElementById('hitcounter');
	if (!el) return;
	var base = 13375; // every good 2005 site launched mid-count
	var key = 'cwr_hits';
	var n;
	try {
		n = parseInt(localStorage.getItem(key) || '0', 10);
		if (!n) { n = base + Math.floor((new Date()).getTime() % 421); }
		n += 1;
		localStorage.setItem(key, String(n));
	} catch (e) { n = base; }
	var s = ('0000000' + n).slice(-7);
	el.innerHTML = '';
	for (var i = 0; i < s.length; i++) {
		var d = document.createElement('span');
		d.className = 'counter';
		d.style.marginRight = '1px';
		d.appendChild(document.createTextNode(s.charAt(i)));
		el.appendChild(d);
	}
}

/* ---- 2. Live "last updated" + local clock in the status area ---- */
function initClock() {
	var el = document.getElementById('clock');
	if (!el) return;
	function tick() {
		var d = new Date();
		function p(x){ return (x < 10 ? '0' : '') + x; }
		el.firstChild
			? (el.firstChild.nodeValue = p(d.getHours())+':'+p(d.getMinutes())+':'+p(d.getSeconds())+' LOCAL')
			: el.appendChild(document.createTextNode('--:--:--'));
	}
	el.appendChild(document.createTextNode('--:--:--'));
	tick();
	setInterval(tick, 1000);
}

/* ---- 3. Marquee fallback for browsers that dropped <marquee> ---- */
function initTicker() {
	var m = document.getElementById('ticker');
	if (!m) return;
	// If native <marquee> works, leave it. Otherwise JS-scroll a span.
	if ('start' in m) return; // native marquee present
	var span = m.getElementsByTagName('span')[0];
	if (!span) return;
	var x = m.offsetWidth;
	setInterval(function () {
		x -= 2;
		if (x < -span.offsetWidth) x = m.offsetWidth;
		span.style.left = x + 'px';
	}, 30);
}

/* ---- 4. Status-bar message (the classic window.status crawl, revived) ---- */
function initStatusMsg() {
	var el = document.getElementById('statusmsg');
	if (!el) return;
	var msgs = [
		'>> Selecting mission...',
		'>> DEFCON 3 -- stand by',
		'>> Radio check, over.',
		'>> Everon: partly cloudy, wind SW 4 m/s',
		'>> All units, hold this position.',
		'>> Contact! Grid reference incoming...',
		'>> Rearming at supply truck.'
	];
	var i = 0;
	setInterval(function () {
		i = (i + 1) % msgs.length;
		// replace all child text with the new message
		while (el.firstChild) el.removeChild(el.firstChild);
		el.appendChild(document.createTextNode(msgs[i]));
	}, 3500);
}

/* ---- 5. Animate the design-loop progress bars on load ---- */
function initBars() {
	var bars = document.getElementsByClassName('pbar');
	for (var i = 0; i < bars.length; i++) {
		var span = bars[i].getElementsByTagName('span')[0];
		if (!span) continue;
		var target = span.getAttribute('data-pct') || '0';
		span.style.width = '0%';
		(function (s, t) {
			var cur = 0, goal = parseInt(t, 10);
			var iv = setInterval(function () {
				cur += 2;
				if (cur >= goal) { cur = goal; clearInterval(iv); }
				s.style.width = cur + '%';
			}, 20);
		})(span, target);
	}
}

/* ---- 6. "Best viewed" nag only once per session ---- */
function initNag() {
	var el = document.getElementById('nag');
	if (!el) return;
	try {
		if (sessionStorage.getItem('cwr_nag')) { el.style.display = 'none'; return; }
		sessionStorage.setItem('cwr_nag', '1');
	} catch (e) {}
}

function retroInit() {
	initCounter();
	initClock();
	initTicker();
	initBars();
	initNag();
	var sm = document.getElementById('statusmsg');
	// statusmsg node handling: find text node
}

if (window.addEventListener) window.addEventListener('load', retroInit, false);
else if (window.attachEvent) window.attachEvent('onload', retroInit);
