<?php
// -----------------------
// buildgallery.php v1.9.1
// ----------------------
//
// by:
// Jack Hardie - www.jhardie.com
// Mario - mariohm@fibertel.com.ar
// Mike Peck - www.mikecpeck.com
// Mike Johnson - mike@solanosystems.com
// Christian Machmeier - www.redsplash.de
// Airtight - www.airtightinteractive.com
//
// DESCRIPTION
// -----------------------
// This script automatically generates the XML document and thumbnails for SimpleViewer
// www.airtightinteractive.com/simpleviewer/
//
// TO USE
// -----------------------
// Instructions are at: www.airtightinteractive.com/simpleviewer/auto_server_instruct.html
//
//
// SET GALLERY OPTIONS HERE
// -----------------------
// Set Gallery options by editing the values in "double quotes" below:

// -----------------------------------------------------------------------------------------------
// Class Options
// contains options for simpleviewer and buildgallery
class Options
{
// array svOptions simpleviewer options
  var $svOptions;
// array bgOptions buildgallery options
  var $bgOptions;
// string gdVersion version number of GD library
  var $gdVersion;
// floating point number gdVersionNumber version number of GD library expressed as a number
// contents of $gdVersion after the second decimal point will be lost
// function Options constructs Options pre-processes path data and gets GD version

  function Options($svOptions, $bgOptions)
  {
    $this->svOptions = $svOptions;
    $this->bgOptions = $bgOptions;
    $this->bgOptions['imagePath'] = $this->getPath('imagePath', $this->svOptions['imagePath']);
    $this->bgOptions['thumbPath'] = $this->getPath('thumbPath', $this->svOptions['thumbPath']);
    $this->gdVersion = $this->getGDVersion();
    $this->gdVersionNumber = (float)$this->gdVersion;

  }
// function getPath extracts image or thumb path from simpleviewer options
  function getPath($pathName, $path)
  {
// handle default path
    if ($path == '' && $pathName == 'imagePath') {return 'images';}
    if ($path == '' && $pathName == 'thumbPath') {return 'thumbs';}
// handle correctly formatted path with trailing /
    $lastChar = substr($path, -1);
    if ($lastChar == '/')
    {
      return substr($path, 0, -1);
    }
    return $path;
  }
  // function getGDVersion
// returns string $gd_version_number
// Use output buffering to get results from phpinfo()
// without disturbing the page we're in.  Output
// buffering is "stackable" so we don't even have to
// worry about previous or encompassing buffering.
  function getGDversion()
  {
    static $gd_version_number = null;
    if ($gd_version_number === null)
    {
      ob_start();
      phpinfo(8);
      $module_info = ob_get_contents();
      ob_end_clean();
      if (preg_match("/\bgd\s+version\b[^\d\n\r]+?([\d\.]+)/i", $module_info,$matches))
      {
        $gd_version_number = $matches[1];
      }
      else
      {
        $gd_version_number = 0;
      }
    }
    return $gd_version_number;
  }
}

// Class ImageFiles
// Extracts information about the files in the designated image directory
class ImageFiles
{
// array $fileNames names and dates of files in images directory
  var $fileNames;
// function ImageFiles constructs ImageFiles
// parameter object $options simpleviewer and buildgallery options
  function ImageFiles($options)
  {
    $this->fileNames = array();
    $docroot = $options->svOptions['docroot'];
    $imageDir = $docroot.$options->bgOptions['imagePath'];
    $trailing_filter = $options->bgOptions['trailingFilter'];
    $verbose = $options->bgOptions['verbose'];
    //echo("ImageFiles: docroot=$docroot tf=$trailing_filter <br>");
    if (!is_dir($imageDir))
    {
      die('<p class="error">Error: the image directory <em>'.$imageDir.'</em> cannot be found</p>');
    }
    $folder = @opendir($imageDir);
    if ($folder === false)
    {
      die('<p class="error">Cannot open the <em>'.$imageDir.'</em> directory &ndash; check the file permissions.</p>');
    }
    $jpgCount = 0;
    $sortMessage = '';
    while(false !== ($fileName = readdir($folder)))
    {
    	if (!$this->isImage($fileName, $trailing_filter)) {continue;}
    	if ($options->bgOptions['sortImagesByDate'])
      {
    			$this->fileNames[$fileName] = filemtime($imageDir.'/'.$fileName);
          $sortMessage = 'Sorting images by date. ';
    	}
      else
      {
    			$this->fileNames[$fileName] = $fileName;
          $sortMessage = 'Sorting images by file name. ';
    	}
      $jpgCount ++;
    }
    if ($jpgCount == 0)
    {
      print '<p class="warning">Warning: no jpg images found in <em>'.$imageDir.'</em> folder';
      if($trailing_filter) print " with trailing $trailing_filter";
      print '.</p>';
      return;
    }
    else
    {
      // print $jpgCount.' jpg images found in <em>'.$imageDir.'</em> folder';
      // if($trailing_filter) print " with trailing $trailing_filter";
      // print '.</br>';
    }
    // now sort by date modified
    if ($options->bgOptions['sortInReverseOrder'])
    {
      $sortMessage .= 'Sort in reverse order.';
    	arsort($this->fileNames, SORT_NUMERIC);
    }
    else
    {
      $sortMessage .= 'Sort in forward order.';
    	asort($this->fileNames, SORT_NUMERIC);
    }
    closedir($folder);
    if($verbose) {
      print '<p>'.$sortMessage.'</p>';
      print '<ol>';
      foreach ($this->fileNames as $fileName => $value)
      {
        print '<li>'.$fileName.'</li>';
      }
      print '</ol>';
    }
  }
// function getFileNames discards date information and returns array of image file names
  function getFileNames()
  {
    return array_keys($this->fileNames);
  }
// function isImage returns true if $fileName has a suffix .jpg or .jpeg, otherwise returns false
// parameter string file name
  function isImage($fileName, $trailing_filter="")
  {
    if ($fileName[0] == "." || $fileName[0] == ".." ) {return false;}
    $components = explode(".", $fileName);
    $length = count($components);
    if ($length == 1) {return false;}
    $suffix = strtolower($components[$length - 1]);
    if($trailing_filter) {
      $last = strtolower($components[$length - 2]);
      $lastn = substr($last, strlen($last)-strlen($trailing_filter));
      //echo("filename=$fileName; tfilter=$trailing_filter last=$last suffix=$suffix lastn=$lastn<br>");
      if($lastn != $trailing_filter) return false;
    }
    if ($suffix == 'jpg' || $suffix == 'jpeg') {return true;}
    return false;
  }
}

// class GalleryXml creates gallery.xml file
class GalleryXml {
// var string $xml contents of gallery.xml file
  var $xml;
// function GalleryXml constructs XmlDoc
// parameter array $fileNames names of image files
// parameter object $options user options
  function GalleryXml($fileNames, $options)
  {
    $this->xml = '<?xml version="1.0" encoding="UTF-8"'." ?>\n";
    $this->xml .= $this->getXmlOptions($options)."\n";
    $verbose = $options->bgOptions['verbose'];
    foreach ($fileNames as  $fileName)
    {
      $this->xml .= "<image>\n";
	    $this->xml .= "\t<filename>".$fileName."</filename>\n";
//add auto captions
	    if ($options->bgOptions['showDownloadLinks'])
      {
		    $this->xml .= "\t".'<caption><![CDATA[<a href="'.$options->bgOptions['imagePath'].'/'.$fileName.'" target="_blank"><u>Open image in new window</u></a>]]></caption>'."\n";
	    }
    	$this->xml .= "</image>\n";
    }
    $this->xml .= '</simpleviewerGallery>';
    $file = "gallery.xml";

    // by kjetil
    $imagepath = $options->bgOptions['imagePath'];
    $docroot = $options->bgOptions['docroot'];
    $file = $imagepath ? "$imagepath/gallery.xml" : "gallery.xml";
    $file = $docroot.$imagepath."/gallery.xml";
    //echo("file = $file<br>");
    // end

// attempt to change permissions
    if (file_exists($file))
    {
      @chmod($file, 0777);
    }
    if (!$file_handle = @fopen($file,"w"))
    {
	    print '<p class="error">Cannot open XML document: '.$file.'. Change permissions to 0777 for $file and parent directory.</p>';
    }
    elseif (!@fwrite($file_handle, $this->xml))
    {
	    print '<p class="error">Cannot write to XML document: '.$file.'. Change permissions to 0777 for file and parent directory.<p>';
    }
    else
    {
      if($verbose) print '<p>Successfully created XML document: <em>'.$file.'</em></p>';
    }
    @fclose($file_handle);
// attempt to change file permissions for later editing by user
    @chmod($file, 0777);
  }
// function getXmlOptions formats simpleviewer options as an xml tag
  function getXmlOptions($options)
  {
    $xmlOptions = '<simpleviewerGallery';
    foreach ($options->svOptions as $optName => $optValue)
    {
      // hack by kjetil to add leading slash to image directories if not already there
      if(($optName=='imagePath' || $optName=='thumbPath') && $optValue[0] != '/') $optValue = "/$optValue";
      $xmlOptions = $xmlOptions .= ' '.$optName.' = "'.$optValue.'"';
    }
    $xmlOptions = $xmlOptions .= '>';
    return $xmlOptions;
  }
}

class Thumbnails
{
// function Thumbnails constructs Thumbnails class
// parameter array $fileNames names of source images
// parameter object $options options for simpleviewer and buildgallery
  function Thumbnails($fileNames, $options)
  {
    //dump($options);
    $imageDir = $options->bgOptions['imagePath'];
    $thumbDir = $options->bgOptions['thumbPath'];
    $docroot =  $options->bgOptions['docroot'];
    $verbose = $options->bgOptions['verbose'];

    $thumbDir=$docroot.$thumbDir;
    $imageDir=$docroot.$imageDir;
    if (!is_dir($thumbDir))
    {
// Note: mkdir($thumbDir, 0777) will not work reliably because of php's umask (www.php.net/umask)
      mkdir($thumbDir);
      chmod($thumbDir, 0777);
    }
    if ($options->gdVersionNumber < 1.8)
    {
      print '<p class="warning">Warning: the GD imaging library was not found on this server or it is an old version that does not support jpeg images. Thumbnails will not be created. Either upgrade to a later version of GD or create the thumbnails yourself in a graphics application such as Photoshop.</p>';
      return;
    }
    elseif ($options->gdVersionNumber < 2)
    {
      print '<p class="warning">Warning: the GD imaging library on this server is version '.$options->gdVersion.'. Thumbnails will be of lower quality. You might want to upgrade to a later version of GD or create the thumbnails yourself in a graphics application such as Photoshop.</p>';
    }
    elseif ($options->bgOptions['useCopyResized'])
    {
      print '<p class="warning">Warning: the <em>useCopyResized</em> setting has been set to <em>true</em>. Thumbnails will be of lower quality. You might want to create the thumbnails yourself in a graphics application such as Photoshop.</p>';
    }
    if($verbose) print '<p>Attempting to create thumbnails in <em>'.$thumbDir.'</em> folder:</p>';
    $thumbCount = 0;
    $thumbList = '<ol>';

    foreach ($fileNames as $fileName)
    {
      $filePath = $imageDir.'/'.$fileName;
      $thumbPath = $thumbDir.'/'.$fileName;
      $thumbCount ++;
      if (file_exists($thumbPath) && !$options->bgOptions['overwriteThumbnails'])
      {
        $thumbList .= '<li>'.$fileName.' already exists</li>';
        continue;
      }
      if ($this->createThumb($filePath, $thumbPath, $options))
      {
        $thumbList .= '<li>'.$fileName.' created</li>';
      }
      else
      {
        $thumbList .= '<li class="warning">'.$fileName." could not be created: filePath=$filePath thumbPath=$thumbPath</li>";
      }
    }
    $thumbList .= '</ol>';
    if ($verbose && $thumbCount > 0) {print $thumbList;}
  }
// function createThumb creates and saves thumbnail image.
// returns boolean $success
// uses older GD library functions if current ones are not available
// parameter string $filePath path to source image
// parameter string $thumbPath path to new thumbnail
// parameter object $options options

  function createThumb($filePath, $thumbPath, $options)
  {
    $thumbSize = 65;
    $quality = 85;
    // Get the image dimensions.
    return resize_image($filePath, $thumbPath, $thumbSize, true, $quality);
  }    
}  

  //   if(!$filePath) {
  //     echo("createThumb: Missing filePath");
  //     return false;
  //   }
  //   
  //   // Get the image dimensions.
  //   list($width, $height, $image_type) = getimagesize($filePath);
  //   $smallerSide = min($width, $height);
  // 
  //   // Calculate offset of square portion of image
  //   // offsets will both be zero if original image is square
  //   $deltaX = ($width - $smallerSide)/2;
  //   $deltaY = ($height - $smallerSide)/2;
  // 
  //   switch ($image_type)
  //   {
  //     case 1: $imageSrc = imagecreatefromgif($filePath); break;
  //     case 2: $imageSrc = imagecreatefromjpeg($filePath);  break;
  //     case 3: $imageSrc = imagecreatefrompng($filePath); break;
  //     default:  trigger_error("Unsupported filetype! path='$filepath' type='$image_type'", E_USER_WARNING);  break;
  //   }
  //   if(!$imageSrc) echo("imagecreatefromXXX failed:  filePath='$filePath'  src='$imageSrc'<br>");
  // 
  //   // Create an empty thumbnail image.
  //   if ($options->gdVersionNumber < 2 || $options->bgOptions['useCopyResized'])
  //   {
  //     $imageDest = @imagecreate($thumbSize, $thumbSize);
  //     $success = @imagecopyresized($imageDest, $imageSrc, 0, 0, $deltaX, $deltaY, $thumbSize, $thumbSize, $smallerSide, $smallerSide);
  //   }
  //   else
  //   {
  //     $imageDest = @imagecreatetruecolor($thumbSize, $thumbSize);
  //     $success = @imagecopyresampled($imageDest, $imageSrc, 0, 0, $deltaX, $deltaY, $thumbSize, $thumbSize, $smallerSide, $smallerSide);
  //   }
  //   if (!$success) {return false;}
  //   {
  //     // save the thumbnail image into a file.
  // 		$success = @imagejpeg($imageDest, $thumbPath, $quality);
  // 		// Delete both image resources.
  // 		@imagedestroy($imageSrc);
  // 		@imagedestroy($imageDest);
  // 	}
  // 	return $success;
  // }


// Class Page produces html headers and footers
class Page
{
// string $title contents for html <title></title> tags
  var $title;
// function Page constructs Page class
  function Page($title)
  {
    $this->title = $title;
  }
// Function getHeader
// returns string containing html header, css styles and page heading
// rules for heredoc ( <<<EOD...EOD; ) are on www.php.net/heredoc
// in particular note the rules for white space in closing heredoc tags
  function getHeader()
  {
    $header = <<<EOD
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title>$this->title</title>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />

<style type="text/css">
  body {
    background-color: #ffffff;
    color: #333333;
    font-family: arial, helvetica, sans-serif;
    font-size: 62.5%;
  }
  h1 {
    font-size: 1.4em;
    font-weight: bold;
  }
  p, ol {
    font-size: 1.2em;
  }
  .error {
    color: #FF0000;
    background-color: #FFFFFF;
  }
  .warning {
    color: #0000FF;
    background-color: #FFFFFF;
  }
</style>
</head>
<body>
  <h1>Creating XML and thumbnails for SimpleViewer.</h1>
EOD;
  return $header;
  }
// function getFooter returns string containing closing html tags
// could also be used for a page footer, such as a copyright message
// rules for heredoc ( <<<EOD...EOD; ) are on www.php.net/heredoc
// in particular note the rules for white space in closing heredoc tags
  function getFooter()
  {
    $footer = <<<EOD
</body>
</html>
EOD;
  return $footer;
  }
}

class Smallpics
{
// function Thumbnails constructs Thumbnails class
// parameter array $fileNames names of source images
// parameter object $options options for simpleviewer and buildgallery
  function Smallpics($fileNames, $options)
  {
    //dump($options);
    $imageDir = $options->bgOptions['imagePath'];
    $smallDir = $options->bgOptions['smallPath'];
    $docroot =  $options->bgOptions['docroot'];
    $verbose = $options->bgOptions['verbose'];
    $smallDir=$docroot.$smallDir;
    $imageDir=$docroot.$imageDir;
    if (!is_dir($smallDir))
    {
// Note: mkdir($smallDir, 0777) will not work reliably because of php's umask (www.php.net/umask)
      mkdir($smallDir);
      chmod($smallDir, 0777);
    }
    if ($options->gdVersionNumber < 1.8)
    {
      print '<p class="warning">Warning: the GD imaging library was not found on this server or it is an old version that does not support jpeg images. Smallnails will not be created. Either upgrade to a later version of GD or create the smallnails yourself in a graphics application such as Photoshop.</p>';
      return;
    }
    elseif ($options->gdVersionNumber < 2)
    {
      print '<p class="warning">Warning: the GD imaging library on this server is version '.$options->gdVersion.'. Smallnails will be of lower quality. You might want to upgrade to a later version of GD or create the smallnails yourself in a graphics application such as Photoshop.</p>';
    }
    elseif ($options->bgOptions['useCopyResized'])
    {
      print '<p class="warning">Warning: the <em>useCopyResized</em> setting has been set to <em>true</em>. Smallnails will be of lower quality. You might want to create the smallnails yourself in a graphics application such as Photoshop.</p>';
    }
    if($verbose) print '<p>Attempting to create smallpics in <em>'.$smallDir.'</em> folder:</p>';
    $smallCount = 0;
    $smallList = '<ol>';

    $failed=$created=0;
    foreach ($fileNames as $fileName)
    {
      $filePath = $imageDir.'/'.$fileName;
      $smallPath = $smallDir.'/'.$fileName;
      $smallCount ++;

      if(!$filePath) {
        echo("smallPics: Missing filePath: fileName='$fileName' imageDir='$imageDir'");
        dump($fileNames);
        return false;
      }

      if (file_exists($smallPath) && !$options->bgOptions['overwriteSmallnails'])
      {
        $smallList .= '<li>'.$fileName.' already exists</li>';   
        //echo("Overwrite not set - skipping create small<br>");
        continue;
      }
      //echo("creating small<br>");

      if ($this->createSmall($filePath, $smallPath, $options))
      {
        $smallList .= '<li>'.$fileName.' created</li>';
        $created++;
      } else {
        $smallList .= '<li class="warning">'.$fileName." could not be created: filePath=$filePath smallPath=$smallPath</li>";
        $failed++;
      }
    }
    $smallList .= '</ol>';
    if ($verbose && $smallCount > 0) {print $smallList;}
    if($created) echo("Created $created smallpics for in $smallPath<br>");
    if($failed) echo("<b> Failed to create $failed smallpics for in $smallPath</b>$smallList<br>");
  }
// function createSmall creates and saves smallnail image.
// returns boolean $success
// uses older GD library functions if current ones are not available
// parameter string $filePath path to source image
// parameter string $smallPath path to new smallnail
// parameter object $options options

  function createSmall($filePath, $smallPath, $options) {
    $smallSize = 120;
    $quality = 100;
    return resize_image($filePath, $smallPath, $smallSize, true, $quality);
  }
}



?>