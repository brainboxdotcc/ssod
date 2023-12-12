#!/usr/bin/perl

#
# Updates hourly stats on numver of users, runs off crontab once an hour
#

use DBI;

my $dbh = DBI->connect('DBI:mysql:brain:localhost','brain','wjr84lik',{ RaiseError => 1});
if (!$dbh)
{
	die "Cannot connect to database";
}
my $query = "SELECT count(username) FROM game_users WHERE username != '**NONE**' AND lastuse > (UNIX_TIMESTAMP() - 600) ORDER BY username";
my $sth = $dbh->prepare($query);
if (!$sth)
{
	die "Illegal query: $query";
};
$sth ->execute;
while (my @row = $sth->fetchrow_array) {
  $usercount = $row[0];
}
$sth->finish;

my $currenthour=`date +%k`;
 
$query = "UPDATE game_hourly_stats SET users=$usercount WHERE hour=$currenthour";
$sth = $dbh->prepare($query);
if (!$sth)
{
	die "Illegal query: $query";
};
$sth ->execute;
$sth->finish;
