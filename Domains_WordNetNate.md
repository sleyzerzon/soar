# WordNet for Soar #

This project is a word sense disambiguation task that involves some preliminary work importing a [WordNet](http://wordnet.princeton.edu) database into Soar's Semantic Memory.  It contains a set of PhP scripts that does various conversions to a format that Soar can use and an agent that uses that knowledge to disambiguate words in various sentences.

### Download Links ###
  * [WordNet.zip](http://web.eecs.umich.edu/~soar/downloads/Domains/WordNet.zip)

### Associated Agents ###
  * Pending

### Documentation ###
#### Data Source ####

We are using data from the WN-LEXICAL project:
  * Project: http://sourceforge.net/projects/actr-wn-lexical/
  * Paper: http://act-r.psy.cmu.edu/publications/pubinfo.php?id=648

From the project page, download the wn-lexical-data for the version of WordNet of interest. Once extracted, WNChunks.data is the file of interest.

#### Conversion ####

The data files in this project contain ACT-R chunks. The conversion to Soar's semantic memory declarative add is basically string replacement. We have a PHP script to perform this conversion on our [git repository](https://github.com/SoarGroup/Domains-WordNet). The script takes via standard-in a data file from WN-LEXICAL and outputs a valid semantic memory declarative add statement. For example:

cat WNChunks.data | php wnlexical.php smem > wn.soar

This conversion is very fast (~30s) and produces about 84MB of text data. Sourcing the resulting data into Soar takes a few minutes and a good amount of memory (~1GB). To make this a one-time cost, switch semantic memory to write to a file before sourcing the data:

```
smem --set database file
smem --set path /path/to/wn.db) 
```

This script also can create SQL statements to populate a "sqlite" or "mysql" database (swap the quoted strings for "smem" in the above command). This may be useful to search/analyze/understand the WordNet chunks.

#### SemCor ####

We have done some preliminary work with SemCor (http://www.cse.unt.edu/~rada/downloads.html), which is a set of texts semantically annotated with WordNet senses. The scripts described here (available on github) parse the SemCor format (cleaning as necessary), cross-reference the SQL outputs of the WN conversion above, and create WordNet-independent Word Sense Disambiguation (WSD) test sets.

  * semcor.php:
    * input=semcor
    * output=cleaned, non-sgml format
  * semcor-tags.php:
    * input=semcor taglist directory
    * output=output of semcor.php run on all files in directory
  * wn-semcor.php:
    * input=SQLite3 database produced from wnlexical.php, output of semcor.php
    * output=SQL for WSD database

Example sequence (assumes WN-LEXICAL 3 and SemCor 3):

  1. php wnlexical.php sqlite WNChunks.data | sqlite3 wn.db
  1. php semcor.php < br-a01 > br-a01.txt
  1. php wn-semcor.php wn.db sqlite 1 y br-a01.txt | sqlite3 br-a01.db

Note that for all but the very smallest SemCor data files, additional indexes on the WordNet database will dramatically reduce production of the WSD database (step 3):

  * CREATE INDEX sk\_sk ON wn\_chunk\_sk (sense\_key);
  * CREATE INDEX s\_syn\_w\_sn\_st\_wl ON wn\_chunk\_s (synset\_id,w\_num,sense\_number,ss\_type,word\_lower);
  * CREATE INDEX s\_wl\_st ON wn\_chunk\_s (word\_lower,ss\_type);
  * CREATE INDEX g\_syn ON wn\_chunk\_g (synset\_id);

The output will contain four tables:

  * wsd\_sentences:
    * c\_id=corpus id (from step 3)
    * s\_id=sentence id (from SemCor)
    * w\_id=word id (from SemCor)
    * w\_lex=lexical word to search (from WordNet)
    * w\_pos=part-of-speech (from SemCor/WordNet; note "a" covers both adjectives and satellites in WN)
  * wsd\_word\_options:
    * c\_id, s\_id, w\_id = from wsd\_sentences
    * w\_synset = possible synonym set (from WordNet)
    * w\_tag\_count = corpus frequency of synset (from WordNet)
    * w\_gloss = synset definition (from WordNet)
  * wsd\_word\_assignments
    * c\_id, s\_id, w\_id = from wsd\_sentences
    * w\_synset = correct assignment (from SemCor, could be multiple)
  * wsd\_ambiguity
    * c\_id, w\_pos = from wsd\_sentences
    * ambig = (# rows in wsd\_assignments)/(# rows in wsd\_options)
    * ambig\_prop = proportion of w\_pos in c\_id with this ambig value

### Associated Publications ###

### Developer ###
Nate Derbinsky

### Soar Versions ###
Any

### Language ###
PHP