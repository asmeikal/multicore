#!/usr/bin/awk

{
	total[$1] += $3 * 60 + $4
	quad[$1] += ($3 * 60 + $4)^2
	count[$1]++
}
END {
	print "threads media deviazione speedup"
	t1 = total[1] / count[1]
	for (x in count) {
		if (x == "threads") continue
		qmed = (quad[x]/count[x])
		amed = (total[x]/count[x])
		spdp = (t1 * count[x])/total[x]
		print x " " amed " " sqrt(qmed - amed^2) " " spdp
	}
}


