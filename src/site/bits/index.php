<?php
    $app['releasever'] = "v1.4 (Elara Edition)";
    $app['releasedate'] = "22nd March 2013";
    $app['releasefile'] = "redeclipse_1.4";
    $app['background'] = "/bits/background_01.jpg";
    $app['youtubevid'] = "oJRZHjyj7Zg";
    $app['screenshots'] = 84;

    $app['banner'] = "<b>New Release:</b> ". $app['releasedate'] ." <i>". $app['releasever'] ."</i>";
    $app['bannerurl'] = "/download";

    $app['targets'] = array('home' => array('name' => '', 'url' => '/', 'alturl' => '', 'nav' => -1, 'redir' => 0));

    // nav items should be in reverse order for the top navbar 
    $app['targets']['download'] = array('name' => 'Download', 'url' => 'http://sourceforge.net/projects/redeclipse/files/'.$app['releasefile'].'/', 'alturl' => 'http://sourceforge.net/projects/redeclipse/files/', 'nav' => 1, 'redir' => 1);
    $app['targets']['tracker'] = array('name' => 'Tracker', 'url' => 'http://forum.freegamedev.net/viewtopic.php?f=74&t=5192', 'alturl' => 'http://forum.freegamedev.net/viewtopic.php?f=74&t=5192', 'nav' => 1, 'redir' => 1);
    $app['targets']['chat'] = array('name' => 'Chat', 'url' => 'http://webchat.freenode.net/?channels=redeclipse', 'alturl' => '', 'nav' => 1, 'redir' => 1);
    $app['targets']['forum'] = array('name' => 'Forum', 'url' => 'http://forum.freegamedev.net/viewforum.php?f=53', 'alturl' => 'http://forum.freegamedev.net/viewforum.php?f=53&t=', 'nav' => 1, 'redir' => 1);
    $app['targets']['wiki'] = array('name' => 'Wiki', 'url' => 'http://redeclipse.net/wiki/', 'alturl' => 'http://redeclipse.net/wiki/', 'nav' => 1, 'redir' => 1);
    $app['targets']['servers'] = array('name' => 'Servers', 'url' => 'http://redflare.ofthings.net/', 'alturl' => 'http://redflare.ofthings.net/', 'nav' => 1, 'redir' => 1);
    //$app['targets']['ticket'] = array('name' => 'Tickets', 'url' => 'http://redeclipse.net/trac/tickets/', 'alturl' => 'http://redeclipse.net/trac/tickets/', 'nav' => -1, 'redir' => 1);
    $app['targets']['svn'] = array('name' => 'SVN', 'url' => 'http://svn.icculus.org/redeclipse/', 'alturl' => 'http://svn.icculus.org/redeclipse/', 'nav' => -1, 'redir' => 1);

    $app['targets']['facebook'] = array('name' => 'Facebook', 'url' => 'http://www.facebook.com/redeclipse.net', 'nav' => 0, 'redir' => 1);
    $app['targets']['youtube'] = array('name' => 'Youtube', 'url' => 'http://www.youtube.com/results?search_query=%22Red%20Eclipse%22', 'alturl' => 'http://www.youtube.com/results?search_query=%22Red%20Eclipse%22+', 'nav' => 0, 'redir' => 1);
    $app['targets']['desura'] = array('name' => 'Desura', 'url' => 'http://www.desura.com/games/red-eclipse', 'nav' => 0, 'redir' => 1);

    $app['targets']['google'] = array('name' => 'Google', 'url' => 'http://www.google.com/search?q=%22Red%20Eclipse%22', 'alturl' => 'http://www.google.com/search?q=%22Red%20Eclipse%22+', 'nav' => -1, 'redir' => 1);

    $app['targets']['arch'] = array('name' => 'Arch', 'url' => 'http://aur.archlinux.org/packages.php?ID=47449', 'nav' => -1, 'redir' => 1);
    $app['targets']['chakra'] = array('name' => 'Chakra', 'url' => 'http://www.chakra-project.org/packages/index.php?act=search&subdir=&sortby=date&order=descending&searchpattern=redeclipse', 'nav' => -1, 'redir' => 1);
    $app['targets']['slackware'] = array('name' => 'Slackware', 'url' => 'http://slackbuilds.org/repository/14.0/games/redeclipse/', 'nav' => -1, 'redir' => 1);
    $app['targets']['gentoo'] = array('name' => 'Gentoo', 'url' => 'http://git.overlays.gentoo.org/gitweb/?p=proj/gamerlay.git;a=tree;f=games-fps/redeclipse;h=3487897cbfb5a353ed5dae6add04d888b6cf8ae7;hb=HEAD', 'nav' => -1, 'redir' => 1);

    $app['platforms'] = array('windows' => 0, 'linux' => 0, 'macosx' => 0, 'arch' => 1, 'chakra' => 1, 'slackware' => 1, 'gentoo' => 1);

    $app['targets']['apply'] = array('name' => 'Apply', 'url' => 'http://docs.google.com/forms/d/1ZuvNFCIivYY_b5YLpCa_pnsbdR4oRdPplIkcbBexCDU/viewform', 'nav' => -1, 'redir' => 1);
    $app['targets']['guidelines'] = array('name' => 'Guidelines', 'url' => 'http://redeclipse.net/wiki/Multiplayer_Guidelines', 'nav' => -1, 'redir' => 1);
    $app['targets']['faq'] = array('name' => 'FAQ', 'url' => 'http://redeclipse.net/wiki/FAQ_for_Red_Eclipse', 'nav' => -1, 'redir' => 1);
    $app['targets']['trademark'] = array('name' => 'Trademark', 'url' => 'http://redeclipse.net/wiki/Trademark_Policy', 'nav' => -1, 'redir' => 1);
    $app['targets']['install'] = array('name' => 'Install', 'url' => 'http://redeclipse.net/wiki/How_to_install', 'nav' => -1, 'redir' => 1);
    $app['targets']['devel'] = array('name' => 'Devel', 'url' => 'http://redeclipse.net/wiki/Obtain_development_version', 'nav' => -1, 'redir' => 1);
    $app['targets']['license'] = array('name' => 'License', 'url' => 'http://redeclipse.net/wiki/License', 'nav' => -1, 'redir' => 1);
    $app['targets']['readme'] = array('name' => 'Readme', 'url' => 'http://redeclipse.net/wiki/Readme', 'nav' => -1, 'redir' => 1);

    function checkarg($arg = "", $def = "") {
        return isset($_GET[$arg]) && $_GET[$arg] != "" ? $_GET[$arg] : $def;
    }
    
    $app['target'] = checkarg("target", "home");
    if (!isset($app['targets'][$app['target']])) $app['target'] = "home";

    $title = checkarg("title");
    if ($app['targets'][$app['target']]['redir']) {
        $app['url'] = $title != "" ? (
                $app['targets'][$app['target']]['alturl'] != "" ? $app['targets'][$app['target']]['alturl'].$title : $app['targets'][$app['target']]['url'].$title
        ) : $app['targets'][$app['target']]['url'];
        header("Location: ".$app['url']);
        exit;
    }
    else {
        $app['url'] = $title != "" ? (
                $app['targets'][$app['target']]['alturl'] != "" ? $app['targets'][$app['target']]['alturl'].$title : $app['targets'][$app['target']]['url'].$title
        ) : $app['targets'][$app['target']]['url'];
        $app['navbar'] = ''; // cache the navbar
        foreach ($app['targets'] as $key => $targ) {
            if ($key != "" && $targ['name'] != "" && $targ['nav'] == 1) {
                $app['navbar'] .= '<a href="/'.$key.'">'. $targ['name'] .'</a> ';
            }
        }
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en" dir="ltr">
    <head>
        <title>Red Eclipse: <?php echo ($app['targets'][$app['target']]['name'] != "" ? $app['targets'][$app['target']]['name'] : "Home"); ?></title>
        <meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
        <link rel="shortcut icon" href="/bits/favicon.ico" />
        <link rel="stylesheet" type="text/css" href="/bits/style.css" />
        <script type="text/javascript" src="/bits/js/jquery.js"></script>
        <script type="text/javascript" src="/bits/js/jquery.lightbox-0.5.js"></script>
        <script type="text/javascript">
            $(function() {
                $('#gallery a').lightBox();
            });
        </script>
        <script type="text/javascript" src="/bits/js/easySlider1.7.js"></script>
        <script type="text/javascript">
            $(document).ready(function(){   
                $("#slider").easySlider({
                    auto:           true,
                    continuous:     true,
                    nextId:         "slider1next",
                    prevId:         "slider1prev",
                    prevText: 		"Previous",
                    nextText: 		"Next",
                    controlsShow:	true,
                    controlsBefore:	"",
                    controlsAfter:	"",	
                    controlsFade:	true,
                    firstId: 		"firstBtn",
                    firstText: 		"First",
                    firstShow:		false,
                    lastId: 		"lastBtn",	
                    lastText: 		"Last",
                    lastShow:		false,				
                    vertical:		false,
                    speed: 			500,
                    pause:			10000,
                    numeric: 		false,
                    numericId: 		"controls"
                });
            }); 
        </script>
        <script>
            (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
            (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
            m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
            })(window,document,'script','//www.google-analytics.com/analytics.js','ga');
            ga('create', 'UA-45886576-1', 'redeclipse.net');
            ga('send', 'pageview');
        </script>
    </head>
    <body style="background: url(/bits/bg2.jpg) no-repeat center top, #282828 url(/bits/bg1.jpg) repeat-x" bgcolor="#333">
		<div id="fb-root"></div>
		<script>
				(function(d, s, id) {
				var js, fjs = d.getElementsByTagName(s)[0];
				if (d.getElementById(id)) return;
				js = d.createElement(s); js.id = id;
				js.src = "//connect.facebook.net/en_US/all.js#xfbml=1";
				fjs.parentNode.insertBefore(js, fjs);
				}(document, 'script', 'facebook-jssdk'));
		</script>
<script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0];if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src="//platform.twitter.com/widgets.js";fjs.parentNode.insertBefore(js,fjs);}}(document,"script","twitter-wjs");</script>
        <script type="text/javascript">
  (function() {
    var po = document.createElement('script'); po.type = 'text/javascript'; po.async = true;
    po.src = 'https://apis.google.com/js/plusone.js';
    var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(po, s);
  })();
</script>
		<div id="container">
            <div id="banner"><a href="<?php echo $app['bannerurl']; ?>"><?php echo $app['banner']; ?></a></div>
            <div id="links"><?php echo $app['navbar']; ?></div>
            <div id="header">
                <a href="/home"><img src="/bits/lightbox-blank.gif" alt="Red Eclipse" width="450" height="143" border="0" align="left" title="Red Eclipse" /></a><a href="http://www.cubeengine.com/"><img src="/bits/lightbox-blank.gif" alt="Built on Cube Engine 2" width="150" height="143" border="0" align="right" title="Built on Cube Engine 2" /></a>
            </div>
            <div id="video">
                <div id="main">
                    <h1>Red Eclipse</h1><h2>&nbsp;&nbsp;a Free, Casual Arena Shooter</h2>
                    <h3>Available for Windows, GNU/Linux, BSD, and OSX</h3>
                    <h3>Parkour, impulse boosts, dashing, and other tricks</h3>
                    <h3>Favourite game modes with an array of mutators and variables</h3>
                    <h3>Builtin editor lets you create your own maps cooperatively online</h3>
                    <a href="/download" id="button">Free Download<br /><em><?php echo $app['releasever']; ?><br /> released <i><?php echo $app['releasedate']; ?></i></em></a>	 
                    <p id="digidist">also available on <a href="/desura">Desura</a>, for Windows/Linux</p>
                    <p id="mirror">and 3<sup>rd</sup> party packages for:
<?php                   $m = 0;
                        foreach ($app['platforms'] as $key => $targ) {
                            if ($key != "" && $targ == 1) {
                                if ($m == 1) {
                                    echo ", ";
                                    $m = 0;
                                }
                                echo "<a href=\"". $app['targets'][$key]['url'] ."\">". $app['targets'][$key]['name'] ."</a>";
                                $m = 1;
                            }
                        }
                        if ($m == 1) {
                            echo ".";
                            $m = 0;
                        } ?>
                    </p>
                    <p id="svn">or, get the <a href="/devel">development version</a> and live on the bleeding edge</p>
                </div>
                <div id="player">
                    <object width="500" height="308" type="application/x-shockwave-flash" data="http://www.youtube.com/v/<?php echo $app['youtubevid']; ?>&amp;color1=0x000000&amp;color2=0x000000&amp;border=0&amp;fs=1&amp;egm=0&amp;showsearch=0&amp;showinfo=0&amp;ap=%2526fmt%3D18">
                        <param name="movie" value="http://www.youtube.com/v/<?php echo $app['youtubevid']; ?>&amp;color1=0x000000&amp;color2=0x000000&amp;border=0&amp;fs=1&amp;egm=0&amp;showsearch=0&amp;showinfo=0&amp;ap=%2526fmt%3D18" />
                        <param name="allowscriptaccess" value="always" />
                        <param name="allowFullScreen" value="true" />
                    </object>
                </div>
            </div>
            <div class="endblock">&nbsp;</div>
            <div class="sliderblock">
                <div id="slider">
                    <ul id="gallery">
<?php                   $i = 1;
                        $c = true;
                        while ($app['screenshots'] >= $i) {
                            $j = $i%4;
                            if ($j == 1) {
                                echo "<li>";
                                $c = false;
                            }
                            $k = $i < 10 ? "00".$i."" : "0".$i."";
                            echo "<a href=\"/bits/images/".$k.".jpg\"><img src=\"/bits/thumbs/".$k.".jpg\" width=\"180\" height=\"102\" border=\"0\" alt=\"Screenshot ".$k."\" /></a>";
                            if ($j == 4) {
                                echo "</li>";
                                $c = true;
                            }
                            $i++;
                        }
                        if ($c != true) { echo "</li>"; } ?>
                    </ul>
                </div>
            </div>
            <div class="endblock">&nbsp;</div>
            <div class="leftcol">
                <p>Red Eclipse is a fun-filled new take on the casual first person arena shooter, built as a total conversion of <a href="http://www.cubeengine.com/">Cube Engine 2</a>, which lends itself toward a balanced gameplay, with a general theme of agility in a variety of environments. For more information, please see our <a href="/wiki">Wiki</a> or <a href="/forum">Forums</a>.</p>
                <p>The project is a <i>Free and Open Source</i> game, built on <a href="http://www.cubeengine.com/">Cube Engine 2</a> using <a href="http://libsdl.org/">SDL</a> and <a href="http://opengl.org/">OpenGL</a> which allows it to be ported to many platforms; you can <a href="/download">download a package</a> for <i>Windows, GNU/Linux, BSD, and OSX</i>, or grab a development copy from our <a href="/devel">Subversion</a> repository and live on the bleeding edge.</p>
            </div>
            <div class="vbar">&nbsp;</div>
            <div class="leftcol">
                <p>In a true open source <i>by the people for the people</i> nature, we try to work closely with the gaming and open source communities to provide a better overall experience, aiming to create a game environment that is fun and easy to play, while still having elements to master.</p>
                <p>If you think you might have something to contribute to the game or community, please feel free to drop by our <a href="/chat">Chat</a> or <a href="/forum">Forums</a> and talk to us directly. We try to maintain a standard of friendly behaviour in our community, so don't be afraid to speak up and have your say in building this game for us all <b>:)</b></p>
            </div>
            <div class="rightblock">
                <h4>Support Us</h4>
                <p id="donatemsg">Red Eclipse is developed by volunteers, and you get it free of charge; your contributions keep this project alive. You're encouraged to get involved with the community and help evolve the game.</p>
            </div>
            <div class="endrightblock">&nbsp;</div>
            <div id="footer">
                <a href="/download">Download</a>, <a href="/chat">Chat</a>, <a href="/forum">Discuss</a>, <a href="/wiki">Learn More</a>, or <a href="/tracker">Report a Bug</a> today.
            </div>
            <div id="copyright" align="center">
                <p>Red Eclipse, Copyright (C) 2009-2013 Quinton Reeves, Lee Salzman</p>
                <p>Cube Engine 2, Copyright (C) 2001-2013 Wouter van Oortmerssen, Lee Salzman, Mike Dysart, Robert Pointon, and Quinton Reeves</p>
            </div>
        </div>
    </body>
</html>
<?php } ?>
