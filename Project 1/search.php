<!DOCTYPE html>
<head>
	<meta http-equiv="Content-type" content="text/html; charset=utf-8" />
	<title>Project1</title>
	<link rel = "stylesheet" type = "text/css" href = "yee.css" />
</head>
<body>
	<div>
		<center>
			<h1>Project1</h1>
			<form method="GET" action="search.php">
				<input type="text" name="key" value="<?php echo $_GET[key] ?>">
				<input type="submit" value="Go!">
				<br />
				<br />
			</form>
		</center>
	</div>
	<?php
		$source = "out";
		if ( empty($_GET[key]) )
		{
			$source = "opt";
		}
		else
		{
			$query = 'grep "' . $_GET[key] . '" opt';
			$out = shell_exec($query);
			file_put_contents('out', $out);
		}

		# number 1
		$total_items = shell_exec('wc -l ' . $source);

		# number 2
		$perpage = 100;

		# number 3
		$total_pages = ceil($total_items / $perpage);

		# number 4 (you have to decide on the _GET parameter you are going to use, like page.php?page=2 etc.
		$current_page = ($_GET['page'] == '') ? 1:$_GET['page'];

		# number 5
		$offset = ($current_page - 1) * $perpage;
		$query = 'sed -n \''. ($offset + 1) . ',' . ($offset + $perpage) . 'p\' ' . $source;
		$out = shell_exec($query);

		$entries = preg_split('/\n/', trim($out));
	?>
	<center>
		<table class="main">
			<tbody>
				<?php
					foreach ($entries as $entry) {
						echo '<tr class="row">';
							echo '<td>';
								echo $entry . '<br />';
							echo '</td>';
						echo '</tr>';
					}
				?>
			</tbody>
		</table>
	</center>

	<?php
		# number 6
		$up = $_GET[page] + 5;
		$low = $_GET[page] - 4;
		if ( $low < 1 ) {
			$up = $up - $low + 1;
			$up = $up < $total_pages ? $up : $total_pages;
			$low = 1;
		}
		else if ( $up > $total_pages )
		{
			$low = $low - $up + $total_pages;
			$low = $low > 1 ? $low : 1;
			$up = $total_pages;
		}
	?>
	<div style="text-align: center">
	<?php
		echo '<a class="page" href="search.php?key=' . $_GET[key] . '&page=1">1</a> ';
		for ($i = $low + 1; $i <= $up - 1; $i++) {
			echo '<a class="page" href="search.php?key=' . $_GET[key] . '&page='.$i.'">'.$i.'</a> ';
		}
		echo '<a class="page" href="search.php?key=' . $_GET[key] . '&page='.$total_pages.'">'.$total_pages.'</a> ';
	?>
	</div>
	<div>
		<center>
			<form method="GET" action="search.php">
				<br>
				<input type="text" name="page" value="<?php echo $_GET[page] ?>">
				<input hidden type="text" name="key" value="<?php echo $_GET[key] ?>">
				<br /><br />
				<input type="submit" value="Jump to page!">
			</form>
		</center>
	</div>
</body>
