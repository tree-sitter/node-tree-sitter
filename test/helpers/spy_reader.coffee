module.exports =
class SpyReader
  constructor: (@string, @chunkSize) ->
    @chunksRead = []
    @position = 0

  seek: (@position) ->

  read: ->
    start = @position
    @position += @chunkSize
    result = @string.slice(start, @position)
    @chunksRead.push(result)
    result
