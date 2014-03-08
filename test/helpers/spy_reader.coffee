module.exports =
class SpyReader
  constructor: (@string, @chunkSize) ->
    @chunksRead = []
    @position = 0

  seek: (@position) ->
    console.log "position: ", @position

  read: ->
    start = @position
    @position += @chunkSize
    result = @string.slice(start, @position)

    console.log 'in read:', result
    @chunksRead.push(result)
    result
