#@ONERR_DIE|Database @DB@ does not exists|
USE @DB@//


#@IF|$id_type == 1//
#@ONERR_IGNORE||
DROP VIEW IF EXISTS @DB@.@TABLE@_htm_@HTMDEPTH@//

#@ONERR_DIE|Cannot create view @TABLE@_htm_@HTMDEPTH@|
CREATE ALGORITHM=MERGE VIEW @DB@.@TABLE@_htm_@HTMDEPTH@ AS
SELECT @DB@.@TABLE@.*
  FROM DIF.dif INNER JOIN @DB@.@TABLE@ ON (@DB@.@TABLE@.htmID_@HTMDEPTH@=DIF.dif.id)
  WHERE DIF_setHTMDepth(@HTMDEPTH@)  AND  DIF_FineSearch(@RA@, @DEC@, DIF.dif.full)//
#@ENDIF//




#@IF|$id_type == 2//
#@ONERR_IGNORE||
DROP VIEW IF EXISTS @DB@.@TABLE@_healp_@IDOPTTX@_@HEALPORDER@//

#@ONERR_DIE|Cannot create view @TABLE@_healp|
CREATE ALGORITHM=MERGE VIEW @DB@.@TABLE@_healp_@IDOPTTX@_@HEALPORDER@ AS
SELECT @DB@.@TABLE@.*
  FROM DIF.dif INNER JOIN @DB@.@TABLE@ ON (@DB@.@TABLE@.healpID_@IDOPT@_@HEALPORDER@=DIF.dif.id)
  WHERE DIF_setHEALPOrder(@IDOPT@, @HEALPORDER@)  AND  DIF_FineSearch(@RA@, @DEC@, DIF.dif.full)//
#@ENDIF//
